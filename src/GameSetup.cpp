#include "GameSetup.h"

void GameSetup::CreateDebugWindow()
{
    std::shared_ptr<GameWatcher> watcher = GetOrCreateWatcherAndStartPolling();
    std::unique_ptr<DebugGameStateWindow> debugWindow = std::make_unique<DebugGameStateWindow>(watcher);

    allDebugWindows.emplace_back(std::move(debugWindow));
}

SimpleTimerWindow& GameSetup::CreateSimpleTimer()
{
    std::unique_ptr<SimpleTimerWindow> timerWindow = std::make_unique<SimpleTimerWindow>(false);

    allSimpleTimers.emplace_back(std::move(timerWindow));
    return *allSimpleTimers.back().get();
}

std::shared_ptr<GameWatcher> GameSetup::onWatcherTimerUpdate()
{
    std::shared_ptr<GameWatcher> watcher = watcherToPoll.lock();
    if (!watcher)
    {
        mainPollTimer->stop();
        mainPollTimer.reset();
        return {};
    }

    //TODO: notifications for when we lose game state

    watcher->PollGameState();

    if (watcher->IsReady())
    {
        for (std::unique_ptr<SimpleTimerWindow> &timerWindow : allSimpleTimers)
            timerWindow->RefreshStateFromWatcher();
    }

    // always update debug stuff, for our own sanity
    for (std::unique_ptr<DebugGameStateWindow> &debugWindow : allDebugWindows)
        debugWindow->RefreshStateFromWatcher();

    // we free and clean out closed windows on the timer tick, rather than in direct response to the window close (to avoid freeing something that's actively making the close callback)
    for (auto i = allSimpleTimers.begin(); i != allSimpleTimers.end(); )
    {
        if (!(**i).isVisible())
            i = allSimpleTimers.erase(i);
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

    return watcher;
}
