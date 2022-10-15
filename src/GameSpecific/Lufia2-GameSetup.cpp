#include "Lufia2-GameSetup.h"

#include "Lufia2-GameWatcher.h"

struct Lufia2GameSetupState
{
};

Lufia2GameSetup::Lufia2GameSetup()
{
    state = std::make_unique<Lufia2GameSetupState>();

    allModes.emplace_back("Ancient Cave - Simple Timer", [this](QWidget *parent)
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(GetOrCreateWatcherAndStartPolling());
        CreateSimpleTimer(parent,
            [=]() { return watcher->GetIntegerValue("floor") == 1; },
            [=]() { return watcher->GetIntegerValue("floor") == 3; },
            [=]() { return watcher->GetIntegerValue("floor") == 0; });
    });
}

Lufia2GameSetup::~Lufia2GameSetup()
{
}

std::string Lufia2GameSetup::Name() const
{
    return "Lufia 2";
}

std::vector<GameSetupMode>& Lufia2GameSetup::Entries()
{
    return allModes;
}

std::shared_ptr<GameWatcher> Lufia2GameSetup::CreateGameSpecificWatcher()
{
    return std::make_shared<Lufia2GameWatcher>();
}
