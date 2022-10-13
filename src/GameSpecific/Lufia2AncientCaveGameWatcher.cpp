#include "Lufia2AncientCaveGameWatcher.h"

bool Lufia2AncientCaveGameWatcher::IsReady()
{
    return snes.HasLocatedRam();
}

void Lufia2AncientCaveGameWatcher::PollGameState()
{
    if (!snes.HasLocatedRam())
    {
        if (!snes.TryLocateRam([&](uint8_t *start, uint8_t *end, uint64_t startAddress)
        {
            //TODO: game ram patterns go here
            return 0;
        }))
            return;
    }

    const MemorySnapshot ram = snes.ReadRam();
    if (ram.AllData.empty())
        return;

    SetIntegerState("floor", ram.ReadInteger<uint8_t>(0x0570));
}
