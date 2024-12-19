#include "Lufia2-GameSetup.h"

#include <algorithm>
#include <iterator>

#include "../GameWatcher.h"
#include "../Widgets/SimpleTimerWindow.h"
#include "../Widgets/NestedTimerWindow.h"
#include "../MemoryWatchers/SnesMemory.h"
#include "../MemoryWatchers/MemoryPatternMatchUtils.h"

namespace
{
    class Lufia2GameWatcher: public GameWatcher
    {
    public:
        bool IsReady() override
        {
            return snes.HasLocatedRam();
        }

        bool ShouldTriggerStart()
        {
            return ((GetFlagValue("OnNameSelect") && GetFlagValue("ScreenFading")) || GetFlagValue("InGruberik")) && !hasFinishedRun;
        }

        bool ShouldTriggerStop()
        {
            int jellyKillCount = GetIntegerValue("JellyKillCount");
            return jellyKillCount == (IsTwoJellyMode ? 2 : 1);
        }

        bool ShouldTriggerReset()
        {
            return !GetFlagValue("OnJellyFloor") && (GetFlagValue("OnTitleMenu") || (GetFlagValue("OnNameSelect") && !GetFlagValue("ScreenFading")));
        }

        bool ShouldProcessFloorChanges()
        {
            return !hasFinishedRun && GetFlagValue("InCave");
        }

        bool IsTwoJellyMode = false;

    protected:
        void PollGameState() override
        {
            // find ram
            lastWarningForUser = "Could not find snes RAM";
            if (!snes.HasLocatedRam())
            {
                if (!snes.TryLocateRam([&](uint8_t *start, uint8_t *end)
                {
                    // We're going to find memory patterns in different areas, then take any offsets that matched both, to reduce false positives
                    std::vector<MemorySearchPattern> ramPatternsA;
                    ramPatternsA.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0xA8, 0x10, 0xB0, 0x29, 0xA8, 0x10, 0x0A, 0x28, 0xCC, 0x1C, 0x41, 0x04, 0xB0, 0x29 }); // Name Select
                    ramPatternsA.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x31, 0x4E, 0x43, 0x3C, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73 }); // Town and Ancient Cave
                    ramPatternsA.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x41, 0x51, 0x3A, 0x28, 0x56, 0x59, 0xA0, 0x2E, 0x39, 0x37, 0x0D, 0x63, 0xEF, 0x3D }); // Overworld
                    std::vector<uint64_t> initialRamOffsets = FindAnyPatternOffsets(ramPatternsA, start, end);

                    if (initialRamOffsets.empty())
                    {
                        lastWarningForUser = "Memory pattern 1 not found";
                        return std::numeric_limits<uint64_t>::max();
                    }

                    std::vector<MemorySearchPattern> ramPatternsB;
                    ramPatternsB.emplace_back(0x00, std::vector<uint8_t>{ 0x09, 0x53, 0x95, 0xF1, 0x00, 0x00, 0x00, 0x00 }); // Town (initially only)
                    ramPatternsB.emplace_back(0x01, std::vector<uint8_t>{ 0x00, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x00 }); // Title Screen and Name Select (sometimes)
                    ramPatternsB.emplace_back(0x19, std::vector<uint8_t>{ 0x2A, 0xA6, 0x7E, 0x29, 0xA6, 0x7E }); // Name Select and stuff (until menu opened)
                    ramPatternsB.emplace_back(0xEC, std::vector<uint8_t>{ 0x9E, 0x03, 0xB0, 0x9E, 0x00, 0x00, 0x00 }); // Cave
                    ramPatternsB.emplace_back(0xEC, std::vector<uint8_t>{ 0x9E, 0x08, 0xB0, 0x9E, 0x00, 0x00, 0x00 }); // Cave
                    std::vector<uint64_t> finalRamOffsets = FilterAdditionalAnyPatternOffsets(initialRamOffsets, ramPatternsB, start, end);

                    //TODO: maybe add UI for picking the right one so the user can select from options to try until they find the right one, if we can't find something more solid to do here?

                    if (finalRamOffsets.empty())
                    {
                        lastWarningForUser = "Memory pattern 2 not found";
                        return std::numeric_limits<uint64_t>::max();
                    }
                    else
                    {
                        if (finalRamOffsets.size() > 1)
                            lastWarningForUser = "Too many memory patterns found";

                        return finalRamOffsets[finalRamOffsets.size() / 2];
                    }
                }))
                    return;
            }

            const MemorySnapshot ram = snes.ReadRam();
            if (ram.AllData.empty())
                return;

            lastWarningForUser.clear();

            // grab data from ram and store it for later use
            bool onTitleMenu = ram.ReadInteger<uint8_t>(0x0009) == 184 && ram.ReadInteger<uint8_t>(0x000A) == 161;
            SetFlagState("OnTitleMenu", onTitleMenu);

            uint16_t locationIdA = ram.ReadInteger<uint16_t>(0x10C00);
            uint16_t locationIdB = ram.ReadInteger<uint16_t>(0x10C02);
            uint16_t locationIdC = ram.ReadInteger<uint16_t>(0x10C04);
            bool inGruberix = locationIdA == 55 && locationIdB == 55 && locationIdC == 125;
            SetFlagState("InGruberik", inGruberix);

            SetFlagState("OnNameSelect", ram.ReadInteger<uint8_t>(0x0009) == 49 && ram.ReadInteger<uint8_t>(0x000A) == 126 && ram.ReadInteger<uint32_t>(0x0011) == 0 && !inGruberix && !onTitleMenu);
            SetFlagState("ScreenFading", !(ram.ReadInteger<uint8_t>(0x0583) == 15 || (ram.ReadInteger<uint8_t>(0x0583) == 128 && ram.ReadInteger<uint8_t>(0x0581) != 0)));

            int bestFloor = ram.ReadInteger<uint8_t>(0x0B75); //NOTE: This is the "best floor" from the scoreboard data, not the actual current floor!
            SetIntegerState("BestFloor", bestFloor);
            int floor = ram.ReadInteger<uint8_t>(0x1E696);
            if (floor > 1) // changes to 0 between floors
                SetIntegerState("Floor", floor - 1);

            bool onOverworld = (locationIdA == 15163 && locationIdB == 15419 && locationIdC == 15164) || // transition
                               (locationIdA == 43176 && locationIdB == 43176 && locationIdC == 43176); // actually there
            SetFlagState("OnOverworld", onOverworld);

            bool inCaveEntrance = locationIdA == 3386 && locationIdB == 3387 && locationIdC == 3388 && !inGruberix;
            SetFlagState("InCaveEntrance", inCaveEntrance);

            bool inCave = locationIdA == 12288 && locationIdB == 12288 && locationIdC == 12288; //NOTE: This changes to 0 between floors, so account for previous state
            inCave |= (locationIdA == 0 && locationIdB == 0 && locationIdC == 0) || GetFlagValue("InCave");
            inCave &= !onTitleMenu && !inGruberix && !onOverworld;
            SetFlagState("InCave", inCave);

            bool onJellyFloor = (locationIdA == 0 && locationIdB == 0 && locationIdC == 0 && floor == 99 && !GetFlagValue("ScreenFading")) || GetFlagValue("OnJellyFloor"); //floor memory value is 99 for both B98 and B99
            SetFlagState("OnJellyFloor", onJellyFloor);

            if (!inCave || inCaveEntrance)
            {
                SetIntegerState("Floor", 0);
                SetFlagState("OnJellyFloor", false);
            }
            else if (onJellyFloor)
                SetIntegerState("Floor", 99);

            uint16_t blobHpAfterCurrentAttack = ram.ReadInteger<uint16_t>(0x162C);
            SetIntegerState("BlobHP", blobHpAfterCurrentAttack);
            uint8_t blobAnimationState = ram.ReadInteger<uint8_t>(0x421d); // also has other values outside of the blob fight sometimes
            bool isBlobDeadAnimation = blobAnimationState == 31;

            if (onJellyFloor)
            {
                bool jellyDeath = GetIntegerValue("BlobHP") == 0 && isBlobDeadAnimation;
                if (GetFlagValue("AllowJellyKillCountChange") && jellyDeath)
                {
                    SetIntegerState("JellyKillCount", GetIntegerValue("JellyKillCount") + 1);
                    SetFlagState("AllowJellyKillCountChange", false);
                }
            }
            else
            {
                SetFlagState("AllowJellyKillCountChange", true);
            }

            if (inCaveEntrance)
                SetIntegerState("CaveRunNumber", GetIntegerValue("JellyKillCount") + 1);

            uint64_t inGameHours = ram.ReadInteger<uint8_t>(0x0b4d);
            uint64_t inGameMinutes = ram.ReadInteger<uint8_t>(0x0b4e);
            uint64_t inGameSeconds = ram.ReadInteger<uint8_t>(0x0b4f);
            uint64_t inGameSubsecondFrames = ram.ReadInteger<uint8_t>(0x0b50);

            if (ShouldTriggerReset())
            {
                hasFinishedRun = false;
                SetIntegerState("JellyKillCount", 0);
            }
            else if (ShouldTriggerStop())
                hasFinishedRun = true;

            if (ShouldTriggerStart() || ShouldProcessFloorChanges())
            {
                if (inGameSubsecondFrames > 50)
                    is60Fps = true;
            }

            if (!hasFinishedRun)
            {
                // the first second of the game time may briefly display wrong on 60fps versions, until we've detected whether we run at 60fps or 50fps; does not affect the actual total time accumulated.
                uint64_t totalMilliseconds = (inGameHours * 60 * 60 + inGameMinutes * 60 + inGameSeconds) * 1000 + 1000 * inGameSubsecondFrames / (is60Fps ? 60 : 50);
                SetIntegerState("InGameMilliseconds", totalMilliseconds);
            }
        }

    private:
        SnesMemory snes;

        bool hasFinishedRun = false;
        bool is60Fps = false; // some versions run at 50fps
    };
}

Lufia2GameSetup::Lufia2GameSetup()
{
    AddGameMode("Ancient Cave - Every 10 Floors", [this]()
    {
        std::vector<std::tuple<int, int, std::string>> allEntries;
        for (int i = 10; i < 100; i += 10)
            allEntries.emplace_back(std::tuple{i - 9, i, "Floors " + std::to_string(i - 9) + "-" + std::to_string(i)});

        allEntries.emplace_back(std::tuple{99, 99, "Jelly Kill"});

        return CreateTimerForFloorSets(allEntries);
    });

    AddGameMode("Ancient Cave - Cores and Archfiends", [this]()
    {
        std::vector<std::tuple<int, int, std::string>> allEntries =
        {
            {11, 13, "Red Cores\r\n(11-13)"},
            {32, 34, "Blue Cores\r\n(32-34)"},
            {41, 43, "Green Cores\r\n(41-43)"},
            {58, 60, "No Cores\r\n(58-60)"},
            {72, 87, "Archfiends\r\n(72-87)"},
            {99, 99, "Jelly Kill"}
        };

        return CreateTimerForFloorSets(allEntries);
    });

    AddGameMode("Ancient Cave - Many Notable Enemies", [this]()
    {
        std::vector<std::tuple<int, int, std::string>> allEntries =
        {
            {7,  9,  "Red Mimics\r\n(7-9)"},
            {11, 13, "Red Cores\r\n(11-13)"},
            {29, 31, "Blue Mimics\r\n(29-31)"},
            {32, 34, "Blue Cores\r\n(32-34)"},
            {37, 39, "Assassins\r\n(37-39)"},
            {41, 43, "Green Cores\r\n(41-43)"},
            {44, 46, "Ninjas\r\n(44-46)"},
            {58, 60, "No Cores\r\n(58-60)"},
            {64, 68, "Gold Golems\r\n(64-68)"},
            {72, 87, "Archfiends\r\n(72-87)"},
            {99, 99, "Jelly Kill"}
        };

        return CreateTimerForFloorSets(allEntries);
    });

    AddGameMode("Ancient Cave - Simple Real Time", [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
        std::unique_ptr<SimpleTimerWindow> timer = std::make_unique<SimpleTimerWindow>();
        timer->SetStartCheck([=]() { return watcher->ShouldTriggerStart(); });
        timer->SetStopCheck([=]() { return watcher->ShouldTriggerStop(); });
        timer->SetResetCheck([=]() { return watcher->ShouldTriggerReset(); });

        return timer;
    });

    AddGameMode("Ancient Cave - Simple Game Time", [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
        std::unique_ptr<SimpleTimerWindow> timerToReturn = std::make_unique<SimpleTimerWindow>();
        SimpleTimerWindow *timer = timerToReturn.get();
        timer->OnRefresh = [=]()
        {
            timer->SetCurrentTime(watcher->GetIntegerValue("InGameMilliseconds"));
        };

        return timerToReturn;
    });

    AddGameBoolOption("Require Two Jelly Kills", [this](bool enabled)
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
        if (watcher) // make not be watching yet!
            watcher->IsTwoJellyMode = enabled;
    }, false);
}

std::unique_ptr<UpdatableGameWindow> Lufia2GameSetup::CreateTimerForFloorSets(const std::vector<std::tuple<int, int, std::string>> &floorSets)
{
    std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
    std::unique_ptr<NestedTimerWindow> timerToReturn = std::make_unique<NestedTimerWindow>();
    NestedTimerWindow *timer = timerToReturn.get();

    for (const auto& [floorNumberMin, floorNumberMax, floorName] : floorSets)
        timer->AddNestedTimer(floorName);

    timer->OnRefresh = [=]()
    {
        if (floorSets.empty())
            return;

        if (watcher->ShouldTriggerStart())
            timer->SetActiveTimer(std::get<2>(floorSets[0]));
        else if (watcher->ShouldTriggerStop())
            timer->StopAllTimers();
        else if (watcher->ShouldTriggerReset())
        {
            timer->ResetAllTimers();
            timer->SetNameDisplayPrefix("", "");
        }
        else if (watcher->ShouldProcessFloorChanges())
        {
            int currentFloor = watcher->GetIntegerValue("Floor");

            std::string targetTimerToActivate;
            std::string targetTimerToFocus;
            for (size_t i = 0; i < floorSets.size(); ++i)
            {
                const auto& [floorNumberMin, floorNumberMax, floorName] = floorSets[i];

                if (i < floorSets.size() - 1)
                {
                    const auto& [floorNumberNextMin, floorNumberNextMax, floorNameNext] = floorSets[i + 1];

                    if (currentFloor > floorNumberMax)
                        targetTimerToActivate = floorNameNext;
                }

                if (currentFloor >= floorNumberMin && currentFloor <= floorNumberMax)
                    targetTimerToFocus = floorName;
            }

            if (!targetTimerToActivate.empty())
                timer->SetActiveTimer(targetTimerToActivate);

            timer->SetFocusTimer(targetTimerToFocus);

            std::string labelPrefix = watcher->IsTwoJellyMode ? std::string("(") + std::to_string(watcher->GetIntegerValue("CaveRunNumber")) + ") " : "";
            if (!targetTimerToFocus.empty())
                timer->SetNameDisplayPrefix(targetTimerToFocus, labelPrefix);
            if (!targetTimerToActivate.empty())
                timer->SetNameDisplayPrefix(targetTimerToActivate, labelPrefix);
        }
    };

    return timerToReturn;
}

std::string Lufia2GameSetup::Name() const
{
    return "Lufia 2";
}

std::shared_ptr<GameWatcher> Lufia2GameSetup::CreateGameSpecificWatcher()
{
    return std::make_shared<Lufia2GameWatcher>();
}
