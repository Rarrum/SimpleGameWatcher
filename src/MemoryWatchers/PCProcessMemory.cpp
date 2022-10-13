#include "PCProcessMemory.h"

struct PCProcessMemoryData
{
};

PCProcessMemory::PCProcessMemory()
{
    data = std::make_unique<PCProcessMemoryData>();
}

std::unique_ptr<PCProcessMemory> PCProcessMemory::FindProcess(std::function<bool(const std::string &processNameLower)> matchProcessName, std::function<bool(const PCProcessMemory &process)> matchProcessMemory)
{
    //TODO
    return std::unique_ptr<PCProcessMemory>(new PCProcessMemory());
}

PCProcessMemory::~PCProcessMemory()
{
}

bool PCProcessMemory::IsStillAlive() const
{
    //TODO
    return false;
}

void PCProcessMemory::ScanMemory(int32_t halfChunkSize, std::function<bool(uint8_t *start, uint8_t *end, uint64_t startAddress)> scanChunk) const
{
    //TODO
}

std::vector<uint8_t> PCProcessMemory::ReadMemory(uint64_t address, uint32_t amount) const
{
    //TODO
    return std::vector<uint8_t>();
}
