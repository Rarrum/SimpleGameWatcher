#include "GameSetup.h"

void GameSetup::StartWatching()
{
    if (watcherToPoll) //should not have been called, already running?
        return;

    watcherToPoll = CreateGameSpecificWatcher();

    mainPollTimer = std::make_unique<QTimer>();
    QObject::connect(mainPollTimer.get(), &QTimer::timeout, [this](){ onWatcherTimerUpdate(); });
    mainPollTimer->start(1000 / 60);
}

void GameSetup::CloseWindowsAndStopWatching()
{
    allNormalWindows.clear();
    allDebugWindows.clear();
    mainPollTimer.reset();

    watcherToPoll.reset();
}

void GameSetup::CreateDebugWindow()
{
    std::unique_ptr<DebugGameStateWindow> debugWindow = std::make_unique<DebugGameStateWindow>(watcherToPoll);

    allDebugWindows.emplace_back(std::move(debugWindow));
}

SimpleTimerWindow* GameSetup::CreateSimpleTimer()
{
    std::unique_ptr<SimpleTimerWindow> timerWindow = std::make_unique<SimpleTimerWindow>(false);

    allNormalWindows.emplace_back(std::move(timerWindow));
    return dynamic_cast<SimpleTimerWindow*>(allNormalWindows.back().get());
}

NestedTimerWindow* GameSetup::CreateNestedTimer()
{
    std::unique_ptr<NestedTimerWindow> timerWindow = std::make_unique<NestedTimerWindow>();

    allNormalWindows.emplace_back(std::move(timerWindow));
    return dynamic_cast<NestedTimerWindow*>(allNormalWindows.back().get());
}

void GameSetup::onWatcherTimerUpdate()
{
    watcherToPoll->PollGameState();

    // we free and clean out user-closed windows on the timer tick, rather than in direct response to the window close (to avoid freeing something that's actively making the close callback)
    for (auto i = allNormalWindows.begin(); i != allNormalWindows.end(); )
    {
        if (!(**i).IsStillOpen())
            i = allNormalWindows.erase(i);
        else
            ++i;
    }

    for (auto i = allDebugWindows.begin(); i != allDebugWindows.end(); )
    {
        if (!(**i).isVisible())
            i = allDebugWindows.erase(i);
        else
            ++i;
    }

    // only update normal windows if the watcher is working
    if (watcherToPoll->IsReady())
    {
        for (std::unique_ptr<UpdatableGameWindow> &normalWindow : allNormalWindows)
            normalWindow->RefreshState();
    }

    // always update debug stuff, for our own sanity
    for (std::unique_ptr<DebugGameStateWindow> &debugWindow : allDebugWindows)
        debugWindow->RefreshState();

    if (OnWatcherUpdate)
        OnWatcherUpdate();
}

std::string GameSetup::SaveLayout()
{
    //TODO
    return "";
}

void GameSetup::RestoreLayout(const std::string &layoutData)
{
    //TODO
}
