#include "Lufia2-GameSetup.h"

#include <algorithm>
#include <iterator>

#include "../GameWatcher.h"
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
            return (GetFlagValue("OnNameSelect") && GetFlagValue("ScreenFading")) || GetFlagValue("InGruberik");
        }

        bool ShouldTriggerStop()
        {
            return GetIntegerValue("Floor") == 99 && GetIntegerValue("BlobHP") == 0 && GetFlagValue("BlobDeathAnimation");
        }

        bool ShouldTriggerReset()
        {
            return GetIntegerValue("Floor") != 99 && (GetFlagValue("OnTitleMenu") || (GetFlagValue("OnNameSelect") && !GetFlagValue("ScreenFading")));
        }

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
            int floor = ram.ReadInteger<uint8_t>(0x0B75);
            SetIntegerState("Floor", floor);

            bool onTitleMenu = ram.ReadInteger<uint8_t>(0x0009) == 184 && ram.ReadInteger<uint8_t>(0x000A) == 161;
            SetFlagState("OnTitleMenu", onTitleMenu);

            uint16_t locationIdA = ram.ReadInteger<uint16_t>(0x10C00);
            uint16_t locationIdB = ram.ReadInteger<uint16_t>(0x10C02);
            uint16_t locationIdC = ram.ReadInteger<uint16_t>(0x10C04);
            bool inGruberix = locationIdA == 55 && locationIdB == 55 && locationIdC == 125;
            SetFlagState("InGruberik", inGruberix);

            SetFlagState("OnNameSelect", ram.ReadInteger<uint8_t>(0x0009) == 49 && ram.ReadInteger<uint8_t>(0x000A) == 126 && ram.ReadInteger<uint32_t>(0x0011) == 0 && !inGruberix && !onTitleMenu);
            SetFlagState("ScreenFading", !(ram.ReadInteger<uint8_t>(0x0583) == 15 || (ram.ReadInteger<uint8_t>(0x0583) == 128 && ram.ReadInteger<uint8_t>(0x0581) != 0)));

            uint16_t blobHpAfterCurrentAttack = ram.ReadInteger<uint16_t>(0x162C);
            SetIntegerState("BlobHP", blobHpAfterCurrentAttack);
            uint8_t blobAnimationState = ram.ReadInteger<uint8_t>(0x421d); // also has other values outside of the blob fight sometimes
            SetIntegerState("BlobAnimationState", blobAnimationState);
            SetFlagState("BlobDeathAnimation", blobAnimationState == 31);
        }

    private:
        SnesMemory snes;
    };
}

Lufia2GameSetup::Lufia2GameSetup()
{
    allModes.emplace_back("Ancient Cave - Simple Timer", [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
        SimpleTimerWindow *timer = CreateSimpleTimer();
        timer->SetStartCheck([=]() { return watcher->ShouldTriggerStart(); });
        timer->SetStopCheck([=]() { return watcher->ShouldTriggerStop(); });
        timer->SetResetCheck([=]() { return watcher->ShouldTriggerReset(); });
    });

    allModes.emplace_back("Ancient Cave - Every 10 Floors", [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
        NestedTimerWindow *timer = CreateNestedTimer();

        for (int i = 10; i < 100; i += 10)
            timer->AddNestedTimer("Floor " + std::to_string(i));

        timer->AddNestedTimer("Jelly Kill");

        timer->OnRefresh = [=]()
        {
            if (watcher->ShouldTriggerStart())
                timer->SetActiveTimer("Floor 10");
            else if (watcher->ShouldTriggerStop())
                timer->StopAllTimers();
            else if (watcher->ShouldTriggerReset())
                timer->ResetAllTimers();
            else
            {
                int floor = watcher->GetIntegerValue("Floor");
                if (floor < 90)
                {
                    int nearest10Floor = (floor / 10) * 10 + 10;
                    timer->SetActiveTimer("Floor " + std::to_string(nearest10Floor));
                }
                else if (floor != 99)
                    timer->SetActiveTimer("Jelly Kill");
            }
        };
    });

    allModes.emplace_back("Ancient Cave - Major Landmarks", [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(Watcher());
        NestedTimerWindow *timer = CreateNestedTimer();

        const std::string nameEnter14 = "Red Cores\r\n(11-13)";
        const std::string nameEnter35 = "Blue Cores\r\n(32-34)";
        const std::string nameEnter44 = "Green Cores\r\n(41-43)";
        const std::string nameEnter61 = "No Cores\r\n(58-60)";
        const std::string nameEnter88 = "Archfiend Freedom\r\n(88+)";
        const std::string nameKillJelly = "Jelly Kill";

        timer->AddNestedTimer(nameEnter14);
        timer->AddNestedTimer(nameEnter35);
        timer->AddNestedTimer(nameEnter44);
        timer->AddNestedTimer(nameEnter61);
        timer->AddNestedTimer(nameEnter88);
        timer->AddNestedTimer(nameKillJelly);

        timer->OnRefresh = [=]()
        {
            if (watcher->ShouldTriggerStart())
                timer->SetActiveTimer(nameEnter14);
            else if (watcher->ShouldTriggerStop())
                timer->StopAllTimers();
            else if (watcher->ShouldTriggerReset())
                timer->ResetAllTimers();
            else
            {
                int floor = watcher->GetIntegerValue("Floor");
                if (floor == 14)
                    timer->SetActiveTimer(nameEnter35);
                else if (floor == 35)
                    timer->SetActiveTimer(nameEnter44);
                else if (floor == 44)
                    timer->SetActiveTimer(nameEnter61);
                else if (floor == 61)
                    timer->SetActiveTimer(nameEnter88);
                else if (floor >= 88 && floor < 99)
                    timer->SetActiveTimer(nameKillJelly);
            }
        };
    });
}

std::string Lufia2GameSetup::Name() const
{
    return "Lufia 2";
}

std::vector<GameSetupMode>& Lufia2GameSetup::Entries()
{
    return allModes;
}

std::shared_ptr<GameWatcher> Lufia2GameSetup::CreateGameSpecificWatcher()
{
    return std::make_shared<Lufia2GameWatcher>();
}
