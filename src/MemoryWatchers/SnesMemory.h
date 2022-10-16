#pragma once

#include <vector>
#include <functional>
#include <memory>

#include "MemorySnapshot.h"

struct SnesMemoryInternal;

class SnesMemory
{
public:
    SnesMemory();
    ~SnesMemory();

    // Scans other processes in the system to try to locate snes rom or ram locations - locator should return max() if not found
    bool TryLocateRam(std::function<uint64_t(uint8_t *start, uint8_t *end)> ramLocator);
    bool TryLocateRom(std::function<uint64_t(uint8_t *start, uint8_t *end)> ramLocator);

    // Returns whether a ram or om locator was successful, or false if the process it was previously located in is gone
    bool HasLocatedRam() const;
    bool HasLocatedRom() const;

    // Reads the entire contents of ram or rom, or empty on failure
    const MemorySnapshot ReadRam() const;
    const MemorySnapshot ReadRom() const;

private:
    std::unique_ptr<SnesMemoryInternal> data;
};
