#pragma once

#include <cstdint>
#include <vector>

struct MemorySearchPattern
{
    MemorySearchPattern() = default;

    inline MemorySearchPattern(uint64_t ramOffset, std::vector<uint8_t> pattern)
    {
        RamOffset = ramOffset;
        Pattern = std::move(pattern);
    }

    uint64_t RamOffset = 0;
    std::vector<uint8_t> Pattern;
};

std::vector<uint64_t> FindAnyPatternOffsets(const std::vector<MemorySearchPattern> &patterns, uint8_t *memoryStart, uint8_t *memoryEnd);
