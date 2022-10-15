#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../GameSetup.h"

struct Lufia2GameSetupState;

class Lufia2GameSetup: public GameSetup
{
public:
    Lufia2GameSetup();
    ~Lufia2GameSetup();
    std::string Name() const override;
    std::vector<GameSetupMode>& Entries() override;

protected:
    std::shared_ptr<GameWatcher> CreateGameSpecificWatcher() override;

private:
    std::vector<GameSetupMode> allModes;
    std::unique_ptr<Lufia2GameSetupState> state;
};
