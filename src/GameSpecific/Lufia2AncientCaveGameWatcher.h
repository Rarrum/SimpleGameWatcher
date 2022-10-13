#pragma once

#include <vector>
#include <string>

#include "../GameWatcher.h"
#include "../MemoryWatchers/SnesMemory.h"

class Lufia2AncientCaveGameWatcher: public GameWatcher
{
public:
    bool IsReady() override;

protected:
    void PollGameState() override;

private:
    SnesMemory snes;
};
