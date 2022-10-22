#include "SnesMemory.h"

#include <limits>

#include "PCProcessMemory.h"

namespace
{
    const uint32_t SnesRamSize = 128 * 1024;
    const uint32_t SnesRomSize = 6 * 1024 * 1024;
}

struct SnesMemoryInternal
{
    std::unique_ptr<PCProcessMemory> snesProcess;
    uint64_t snesRamAddress = 0;
};

SnesMemory::SnesMemory()
{
    data = std::make_unique<SnesMemoryInternal>();
}

SnesMemory::~SnesMemory()
{
}

bool SnesMemory::TryLocateRam(std::function<uint64_t(uint8_t *start, uint8_t *end)> ramLocator)
{
    if (!data->snesProcess || !data->snesProcess->IsStillAlive())
    {
        data->snesRamAddress = 0;

        data->snesProcess = PCProcessMemory::FindProcess([](const std::string &processNameLower)
        {
            return processNameLower.find("snes") != std::string::npos || processNameLower.find("higan") != std::string::npos;
        }, [&](const PCProcessMemory &process)
        {
            process.ScanMemory([&](uint8_t *start, uint8_t *end, uint64_t startAddress)
            {
                uint64_t locatedOffset = ramLocator(start, end);
                if (locatedOffset == std::numeric_limits<uint64_t>::max())
                    return true;
                else
                {
                    data->snesRamAddress = startAddress + locatedOffset;
                    return false;
                }
            });

            return data->snesRamAddress != 0;
        });
    }
    else
    {
        data->snesProcess->ScanMemory([&](uint8_t *start, uint8_t *end, uint64_t startAddress)
        {
            uint64_t locatedOffset = ramLocator(start, end);
            if (locatedOffset == std::numeric_limits<uint64_t>::max())
                return true;
            else
            {
                data->snesRamAddress = startAddress + locatedOffset;
                return false;
            }
        });
    }

    return HasLocatedRam();
}

bool SnesMemory::TryLocateRom(std::function<uint64_t(uint8_t *start, uint8_t *end)> ramLocator)
{
    //TODO
    return false;
}

bool SnesMemory::HasLocatedRam() const
{
    return data->snesProcess && data->snesProcess->IsStillAlive() && data->snesRamAddress != 0;
}

bool SnesMemory::HasLocatedRom() const
{
    //TODO
    return false;
}

const MemorySnapshot SnesMemory::ReadRam() const
{
    MemorySnapshot snapshot;
    if (HasLocatedRam())
         data->snesProcess->ReadMemory(snapshot.AllData, data->snesRamAddress, SnesRamSize);

    return snapshot;
}

const MemorySnapshot SnesMemory::ReadRom() const
{
    //TODO
    return MemorySnapshot();
}
