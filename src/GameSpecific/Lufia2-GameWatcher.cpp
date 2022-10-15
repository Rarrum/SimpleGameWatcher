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
        if (!snes.TryLocateRam([&](uint8_t *start, uint8_t *end, uint64_t startAddress)
        {
            uint64_t patternOffset = 0x320;
            std::vector<MemorySearchPattern> patterns;
            patterns.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0xA8, 0x10, 0xB0, 0x29, 0xA8, 0x10, 0x0A, 0x28, 0xCC, 0x1C, 0x41, 0x04, 0xB0, 0x29 }); // Name Select
            patterns.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x31, 0x4E, 0x43, 0x3C, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73, 0x00, 0x00, 0x9C, 0x73 }); // Town and Ancient Cave
            patterns.emplace_back(0x320, std::vector<uint8_t>{ 0x00, 0x00, 0x41, 0x51, 0x3A, 0x28, 0x56, 0x59, 0xA0, 0x2E, 0x39, 0x37, 0x0D, 0x63, 0xEF, 0x3D }); // Overworld

            return MatchAnyPattern(patterns, start, end, startAddress);
        }))
            return;
    }

    const MemorySnapshot ram = snes.ReadRam();
    if (ram.AllData.empty())
        return;

    // grab data from ram
    SetIntegerState("floor", ram.ReadInteger<uint8_t>(0x0570));
}
