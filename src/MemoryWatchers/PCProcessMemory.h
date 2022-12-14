#pragma once

#include <functional>
#include <vector>
#include <string>
#include <memory>

struct PCProcessMemoryData;

class PCProcessMemory
{
public:
    static std::unique_ptr<PCProcessMemory> FindProcess(std::function<bool(const std::string &processNameLower)> matchProcessName, std::function<bool(const PCProcessMemory &process)> matchProcessMemory);

    PCProcessMemory() = default;
    PCProcessMemory(const PCProcessMemory&) = delete;
    PCProcessMemory(PCProcessMemory &&) = default;
    PCProcessMemory& operator=(const PCProcessMemory&) = delete;
    PCProcessMemory& operator=(PCProcessMemory &&) = default;
    ~PCProcessMemory();

    bool IsStillAlive() const;
    void ScanMemory(std::function<bool(uint8_t *start, uint8_t *end, uint64_t startAddress)> scanChunk) const;
    void ReadMemory(std::vector<uint8_t> &target, uint64_t address, uint32_t amount) const;

private:
    PCProcessMemory(std::unique_ptr<PCProcessMemoryData> &&from);

    std::unique_ptr<PCProcessMemoryData> data;
};
