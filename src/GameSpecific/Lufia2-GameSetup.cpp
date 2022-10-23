#include "Lufia2-GameSetup.h"

#include "Lufia2-GameWatcher.h"

Lufia2GameSetup::Lufia2GameSetup()
{
    allModes.emplace_back("Ancient Cave - Simple Timer", [this]()
    {
        std::shared_ptr<Lufia2GameWatcher> watcher = std::dynamic_pointer_cast<Lufia2GameWatcher>(GetOrCreateWatcherAndStartPolling());
        SimpleTimerWindow *timer = CreateSimpleTimer();
        timer->SetWatcher(watcher);
        timer->SetStartCheck([=]() { return (watcher->GetFlagValue("OnNameSelect") && watcher->GetFlagValue("ScreenFading")) || watcher->GetFlagValue("InGruberik") || watcher->GetIntegerValue("Floor") != 0; });
        timer->SetStopCheck([=]() { return watcher->GetIntegerValue("Floor") == 99 && watcher->GetIntegerValue("BlobHP") == 0 && watcher->GetFlagValue("BlobDeathAnimation"); });
        timer->SetResetCheck([=]() { return watcher->GetFlagValue("OnTitleMenu") || (watcher->GetFlagValue("OnNameSelect") && !watcher->GetFlagValue("ScreenFading")); });
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
