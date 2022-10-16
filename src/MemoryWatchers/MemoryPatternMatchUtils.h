#pragma once

#include <cstdint>
#include <vector>

struct MemorySearchPattern
{
    uint64_t RamOffset = 0;
    std::vector<uint8_t> Pattern;
};

std::vector<uint64_t> FindAnyPatternOffsets(const std::vector<MemorySearchPattern> &patterns, uint8_t *memoryStart, uint8_t *memoryEnd);