#include "GameSetup.h"

const std::vector<GameSetupMode>& GameSetup::Entries()
{
    return allModes;
}

void GameSetup::StartWatching()
{
    if (watcherToPoll) //should not have been called, already running?
        return;

    watcherToPoll = CreateGameSpecificWatcher();

    mainPollTimer = std::make_unique<QTimer>();
    QObject::connect(mainPollTimer.get(), &QTimer::timeout, [this](){ OnWatcherTimerUpdate(); });
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

void GameSetup::AddGameMode(const std::string name, std::function<std::unique_ptr<UpdatableGameWindow>()> creator)
{
    auto createAndStoreWindow = [this,name,creator]()
    {
        std::unique_ptr<UpdatableGameWindow> window = creator();
        allNormalWindows.emplace_back(NormalWindow(name, std::move(window)));
    };

    allModes.emplace_back(GameSetupMode(name, createAndStoreWindow));
}

void GameSetup::OnWatcherTimerUpdate()
{
    watcherToPoll->PollGameState();

    // we free and clean out user-closed windows on the timer tick, rather than in direct response to the window close (to avoid freeing something that's actively making the close callback)
    for (auto i = allNormalWindows.begin(); i != allNormalWindows.end(); )
    {
        if (!i->Window->IsStillOpen())
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
        for (NormalWindow &normalWindow : allNormalWindows)
            normalWindow.Window->RefreshState();
    }

    // always update debug stuff, for our own sanity
    for (std::unique_ptr<DebugGameStateWindow> &debugWindow : allDebugWindows)
        debugWindow->RefreshState();

    if (OnWatcherUpdate)
        OnWatcherUpdate();
}

std::string GameSetup::SaveLayout() const
{
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> layoutData;
    for (const NormalWindow &window : allNormalWindows)
    {
        
    }
    
    //TODO
    return "";
}

void GameSetup::RestoreLayout(const std::string &layoutData)
{
    //TODO
    //json j = json::parse(layoutData);
    //int x = j.get<int>();
}
