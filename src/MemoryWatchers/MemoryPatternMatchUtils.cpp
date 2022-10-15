#include "MemoryPatternMatchUtils.h"

uint64_t MatchAnyPattern(const std::vector<MemorySearchPattern> &patterns, uint8_t *memoryStart, uint8_t *memoryEnd, uint64_t startAddress)
{
    uint64_t searchSize = memoryEnd - memoryStart;
    for (uint64_t currentOffset = 0; currentOffset < searchSize; ++currentOffset)
    {
        for (const MemorySearchPattern &pattern : patterns)
        {
            uint8_t *actualRam = memoryStart + currentOffset;
            uint8_t *patternRam = memoryStart + currentOffset + pattern.RamOffset;
            uint8_t *patternRamEnd = patternRam + pattern.Pattern.size();

            if (patternRamEnd > memoryEnd)
                continue;

            bool allMatches = true;
            while (patternRam < patternRamEnd)
            {
                allMatches = allMatches && (*actualRam == *patternRam);
                ++actualRam;
                ++patternRam;
            }

           if (allMatches)
                return startAddress + currentOffset;
        }
    }
}
