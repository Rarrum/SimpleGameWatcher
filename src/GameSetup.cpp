#include "GameSetup.h"

void GameSetup::CreateDebugWindow(QWidget *parent)
{
    std::shared_ptr<GameWatcher> watcher = GetOrCreateWatcherAndStartPolling();
    DebugGameStateWindow *debugWindow = new DebugGameStateWindow(watcher, parent);
    debugWindow->closeCallback = [this](ClosableQWidget &closingWidget)
    {
        DebugGameStateWindow* hax = (DebugGameStateWindow*)&closingWidget;
        hax->ClearWatcher(); //TODO: Qt is not calling dtor on close!?  Also affects timer... which is leaking!?

        allDebugWindows.remove_if([&](DebugGameStateWindow *existing){ return existing == &closingWidget; });
    };

    allDebugWindows.emplace_back(debugWindow);
}

void GameSetup::CreateSimpleTimer(QWidget *parent, std::function<bool()> shouldStart, std::function<bool()> shouldStop, std::function<bool()> shouldReset)
{
    SimpleTimerState timerState;
    timerState.timer = new SimpleTimerWindow(false, parent);
    timerState.shouldStart = shouldStart;
    timerState.shouldStop = shouldStop;
    timerState.shouldReset = shouldReset;

    timerState.timer->closeCallback = [this](ClosableQWidget &closingWidget)
    {
        allSimpleTimers.remove_if([&](SimpleTimerState &state){ return state.timer == &closingWidget; });
    };

    allSimpleTimers.emplace_back(std::move(timerState));
}

std::shared_ptr<GameWatcher> GameSetup::onWatcherTimerUpdate()
{
    std::shared_ptr<GameWatcher> watcher = watcherToPoll.lock();
    if (!watcher)
    {
        mainPollTimer->stop();
        delete mainPollTimer;
        return {};
    }

    //TODO: notifications for when we lose game state

    if (watcher->IsReady())
    {
        watcher->PollGameState();

        for (SimpleTimerState &timerState : allSimpleTimers)
        {
            if (timerState.shouldStart && timerState.shouldStart())
                timerState.timer->StartTimer();

            if (timerState.shouldStop && timerState.shouldStop())
                timerState.timer->StopTimer();

            if (timerState.shouldReset && timerState.shouldReset())
                timerState.timer->ResetTimer();
        }
    }

    // always update debug stuff, for our own sanity
    for (DebugGameStateWindow *debugWindow : allDebugWindows)
        debugWindow->RefreshState();

    return watcher;
}
