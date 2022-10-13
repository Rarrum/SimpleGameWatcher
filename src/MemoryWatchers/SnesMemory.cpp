#include "SnesMemory.h"

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

bool SnesMemory::TryLocateRam(std::function<uint64_t(uint8_t *start, uint8_t *end, uint64_t startAddress)> ramLocator)
{
    if (!data->snesProcess || !data->snesProcess->IsStillAlive())
    {
        data->snesRamAddress = 0;

        data->snesProcess = PCProcessMemory::FindProcess([](const std::string &processNameLower)
        {
            return processNameLower.find("snes") != std::string::npos;
        }, [&](const PCProcessMemory &process)
        {
            process.ScanMemory(SnesRamSize, [&](uint8_t *start, uint8_t *end, uint64_t startAddress)
            {
                data->snesRamAddress = ramLocator(start, end, startAddress);
                return data->snesRamAddress != 0;
            });

            return data->snesRamAddress != 0;
        });
    }
    else
    {
        data->snesProcess->ScanMemory(SnesRamSize, [&](uint8_t *start, uint8_t *end, uint64_t startAddress)
        {
            data->snesRamAddress = ramLocator(start, end, startAddress);
            return data->snesRamAddress != 0;
        });
    }

    return HasLocatedRam();
}

bool SnesMemory::TryLocateRom(std::function<uint64_t(uint8_t *start, uint8_t *end, uint64_t startAddress)> ramLocator)
{
    return false;
}

bool SnesMemory::HasLocatedRam() const
{
    return data->snesProcess && data->snesRamAddress != 0;
}

bool SnesMemory::HasLocatedRom() const
{
    return false;
}

const MemorySnapshot SnesMemory::ReadRam() const
{
    MemorySnapshot snapshot;
    if (HasLocatedRam())
        snapshot.AllData = data->snesProcess->ReadMemory(data->snesRamAddress, SnesRamSize);

    return snapshot;
}

const MemorySnapshot SnesMemory::ReadRom() const
{
    return MemorySnapshot();
}
