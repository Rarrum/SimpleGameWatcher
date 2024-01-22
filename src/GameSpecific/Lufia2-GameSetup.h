#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../GameSetup.h"

class Lufia2GameSetup: public GameSetup
{
public:
    Lufia2GameSetup();
    std::string Name() const override;
    std::vector<GameSetupMode>& Entries() override;

protected:
    std::shared_ptr<GameWatcher> CreateGameSpecificWatcher() override;

private:
    std::vector<GameSetupMode> allModes;

    void CreateTimerForFloorSets(const std::vector<std::tuple<int, int, std::string>> &floorSets);
};
