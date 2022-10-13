#pragma once

#include <cstdint>
#include <vector>

struct MemorySnapshot
{
    std::vector<uint8_t> AllData;

    template<typename IntType> requires (std::is_integral<IntType>::value)
    IntType ReadInteger(uint64_t offset) const
    {
        size_t size = sizeof(IntType);
        if (offset + size > AllData.size())
            return 0;

        return *(IntType*)(AllData.data() + offset);
    }
};
