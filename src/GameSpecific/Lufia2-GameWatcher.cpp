#include "Lufia2-GameWatcher.h"

#include "../MemoryWatchers/MemoryPatternMatchUtils.h"

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
            std::vector<MemorySearchPattern> ramPatterns;
            ramPatterns.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0xA8, 0x10, 0xB0, 0x29, 0xA8, 0x10, 0x0A, 0x28, 0xCC, 0x1C, 0x41, 0x04, 0xB0, 0x29 }); // Name Select
            ramPatterns.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x31, 0x4E, 0x43, 0x3C, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73 }); // Town and Ancient Cave
            ramPatterns.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x41, 0x51, 0x3A, 0x28, 0x56, 0x59, 0xA0, 0x2E, 0x39, 0x37, 0x0D, 0x63, 0xEF, 0x3D }); // Overworld

            std::vector<uint64_t> ramOffsets = FindAnyPatternOffsets(ramPatterns, start, end);

            //TODO: warn if we find multiple - loading/restoring saved states tend to mess with this, resetting emulator fixes

            if (ramOffsets.empty())
                return std::numeric_limits<uint64_t>::max();
            else
                return ramOffsets[ramOffsets.size() / 2];
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

    SetFlagState("OnNameSelect", ram.ReadInteger<uint8_t>(0x0009) == 49 && ram.ReadInteger<uint8_t>(0x000A) == 126 && !inGruberix && !onTitleMenu);
    SetFlagState("ScreenFading", !(ram.ReadInteger<uint8_t>(0x0583) == 15 || (ram.ReadInteger<uint8_t>(0x0583) == 128 && ram.ReadInteger<uint8_t>(0x0581) != 0)));

    uint16_t blobHpAfterCurrentAttack = ram.ReadInteger<uint16_t>(0x162C);
    SetIntegerState("BlobHP", blobHpAfterCurrentAttack);
    uint8_t blobAnimationState = ram.ReadInteger<uint8_t>(0x421d); // also has other values outside of the blob fight sometimes
    SetIntegerState("BlobAnimationState", blobAnimationState);
    SetFlagState("BlobDeathAnimation", blobAnimationState == 31);
}
