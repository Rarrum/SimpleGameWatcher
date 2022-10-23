#include "Lufia2-GameWatcher.h"

#include "../MemoryWatchers/MemoryPatternMatchUtils.h"

#include <algorithm>
#include <iterator>

bool Lufia2GameWatcher::IsReady()
{
    return snes.HasLocatedRam();
}

void Lufia2GameWatcher::PollGameState()
{
    // find ram
    if (!snes.HasLocatedRam())
    {
        if (!snes.TryLocateRam([&](uint8_t *start, uint8_t *end)
        {
            // We're going to find memory patterns in different areas, then take any offsets that matched both, to reduce false positives
            std::vector<MemorySearchPattern> ramPatternsA;
            ramPatternsA.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0xA8, 0x10, 0xB0, 0x29, 0xA8, 0x10, 0x0A, 0x28, 0xCC, 0x1C, 0x41, 0x04, 0xB0, 0x29 }); // Name Select
            ramPatternsA.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x31, 0x4E, 0x43, 0x3C, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73 }); // Town and Ancient Cave
            ramPatternsA.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x41, 0x51, 0x3A, 0x28, 0x56, 0x59, 0xA0, 0x2E, 0x39, 0x37, 0x0D, 0x63, 0xEF, 0x3D }); // Overworld
            std::vector<uint64_t> ramOffsets = FindAnyPatternOffsets(ramPatternsA, start, end);

            if (ramOffsets.empty())
                return std::numeric_limits<uint64_t>::max();

            std::vector<MemorySearchPattern> ramPatternsB;
            ramPatternsB.emplace_back(0x00, std::vector<uint8_t>{ 0x02, 0x00, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x00 }); // Title Screen and Name Select
            ramPatternsB.emplace_back(0x00, std::vector<uint8_t>{ 0x09, 0x53, 0x95, 0xF1, 0x00, 0x00, 0x00, 0x00 }); // Town (initially only)
            ramPatternsB.emplace_back(0xEB, std::vector<uint8_t>{ 0xB0, 0x9E, 0x03, 0xB0, 0x9E, 0x00, 0x00, 0x00 }); // Cave
            ramOffsets = FilterAdditionalAnyPatternOffsets(ramOffsets, ramPatternsB, start, end);

            //TODO: maybe add UI for picking the right one so the user can select from options to try until they find the right one, if we can't find something more solid to do here?

            if (ramOffsets.empty())
            {
                lastWarningForUser = "Memory pattern not found";
                return std::numeric_limits<uint64_t>::max();
            }
            else
            {
                if (ramOffsets.size() > 1)
                    lastWarningForUser = "Too many memory patterns found";
                else
                    lastWarningForUser.clear();

                return ramOffsets[ramOffsets.size() / 2];
            }
        }))
            return;
    }

    const MemorySnapshot ram = snes.ReadRam();
    if (ram.AllData.empty())
        return;

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
