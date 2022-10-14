#include "Lufia2-GameSetup.h"

#include "Lufia2-GameWatcher.h"

struct Lufia2GameSetupState
{
    std::weak_ptr<Lufia2GameWatcher> sharedWatcher;
};

Lufia2GameSetup::Lufia2GameSetup()
{
    state = std::make_unique<Lufia2GameSetupState>();

    auto getSharedWatcher = [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = state->sharedWatcher.lock();
        if (!watcher)
        {
            watcher = std::make_shared<Lufia2GameWatcher>();
            state->sharedWatcher = watcher;
        }

        return watcher;
    };

    allModes.emplace_back("Ancient Cave - Simple Timer", [=](QWidget *parent)
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = getSharedWatcher();
        //TODO: create timer window and hook it up to watcher!
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
