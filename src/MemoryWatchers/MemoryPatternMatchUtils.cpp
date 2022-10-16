#include "MemoryPatternMatchUtils.h"

std::vector<uint64_t> FindAnyPatternOffsets(const std::vector<MemorySearchPattern> &patterns, uint8_t *memoryStart, uint8_t *memoryEnd)
{
    std::vector<uint64_t> allFoundOffsets;

    uint64_t searchSize = memoryEnd - memoryStart;
    for (uint64_t currentOffset = 0; currentOffset < searchSize; ++currentOffset)
    {
        for (const MemorySearchPattern &pattern : patterns)
        {
            const uint8_t *actualRam = memoryStart + currentOffset + pattern.RamOffset;
            const uint8_t *actualRamEnd = actualRam + pattern.Pattern.size();
            const uint8_t *patternRam = pattern.Pattern.data();
            const uint8_t *patternRamEnd = pattern.Pattern.data() + pattern.Pattern.size();

            if (actualRamEnd > memoryEnd)
                continue;

            bool allMatches = true;
            while (patternRam < patternRamEnd)
            {
                allMatches = allMatches && (*actualRam == *patternRam);
                ++actualRam;
                ++patternRam;
            }

           if (allMatches)
                allFoundOffsets.emplace_back(currentOffset);
        }
    }

    return allFoundOffsets;
}
