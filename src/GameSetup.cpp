#include "GameSetup.h"

GameSetup::~GameSetup()
{
    if (mainPollTimer)
    {
        delete mainPollTimer;
        mainPollTimer = nullptr;
    }
    
    for (SimpleTimerWindow *timerWindow : allSimpleTimers)
        delete timerWindow;
    allSimpleTimers.clear();

    for (DebugGameStateWindow *debugWindow : allDebugWindows)
        delete debugWindow;
    allDebugWindows.clear();
}

void GameSetup::CreateDebugWindow()
{
    std::shared_ptr<GameWatcher> watcher = GetOrCreateWatcherAndStartPolling();
    DebugGameStateWindow *debugWindow = new DebugGameStateWindow(watcher);
    debugWindow->closeCallback = [this](ClosableQWidget &closingWidget)
    {
        DebugGameStateWindow* hax = (DebugGameStateWindow*)&closingWidget;
        hax->ClearWatcher(); //TODO: Qt is not calling dtor on close!?

        allDebugWindows.remove_if([&](DebugGameStateWindow *existing){ return existing == &closingWidget; });
    };

    allDebugWindows.emplace_back(debugWindow);
}

SimpleTimerWindow* GameSetup::CreateSimpleTimer()
{
    SimpleTimerWindow *timerWindow = new SimpleTimerWindow(false);

    timerWindow->closeCallback = [this](ClosableQWidget &closingWidget)
    {
        SimpleTimerWindow* hax = (SimpleTimerWindow*)&closingWidget;
        hax->ClearWatcher(); //TODO: Qt is not calling dtor on close!?

        allSimpleTimers.remove_if([&](SimpleTimerWindow *existing){ return existing == &closingWidget; });
    };

    allSimpleTimers.emplace_back(timerWindow);
    return timerWindow;
}

std::shared_ptr<GameWatcher> GameSetup::onWatcherTimerUpdate()
{
    std::shared_ptr<GameWatcher> watcher = watcherToPoll.lock();
    if (!watcher)
    {
        mainPollTimer->stop();
        delete mainPollTimer;
        mainPollTimer = nullptr;
        return {};
    }

    //TODO: notifications for when we lose game state

    watcher->PollGameState();

    if (watcher->IsReady())
    {
        for (SimpleTimerWindow *timerWindow : allSimpleTimers)
            timerWindow->RefreshStateFromWatcher();
    }

    // always update debug stuff, for our own sanity
    for (DebugGameStateWindow *debugWindow : allDebugWindows)
        debugWindow->RefreshStateFromWatcher();

    return watcher;
}
