#include "GameSetup.h"

#include <nlohmann/json.hpp>

const std::vector<GameSetupMode>& GameSetup::GameModes()
{
    return allModes;
}

std::vector<GameSetupOption>& GameSetup::GameOptions()
{
    return allOptions;
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

void GameSetup::AddGameMode(const std::string &name, std::function<std::unique_ptr<UpdatableGameWindow>()> creator)
{
    auto createAndStoreWindow = [this, name, creator]()
    {
        std::unique_ptr<UpdatableGameWindow> window = creator();
        allNormalWindows.emplace_back(NormalWindow(name, std::move(window)));
        return allNormalWindows.back().Window.get();
    };

    allModes.emplace_back(GameSetupMode(name, createAndStoreWindow));
}

void GameSetup::AddGameBoolOption(const std::string &name, std::function<void(bool enabled)> optionChanged, bool initialState)
{
    allOptions.emplace_back(GameSetupOption(name, optionChanged));
    allOptions.back().Enabled = initialState;
    optionChanged(initialState);
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
    std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> windowData;
    for (const NormalWindow &window : allNormalWindows)
        windowData.emplace(window.GameMode, window.Window->SaveLayout());

    std::unordered_multimap<std::string, bool> optionData;
    for (const GameSetupOption &option: allOptions)
        optionData.emplace(option.Name, option.Enabled);

    nlohmann::json jsonData;
    jsonData["game"] = Name();
    jsonData["windows"] = windowData;
    jsonData["options"] = optionData;
    return jsonData.dump(4);
}

void GameSetup::RestoreLayout(const std::string &layoutData)
{
    allNormalWindows.clear();

    std::string allErrors;
    nlohmann::json jsonData = nlohmann::json::parse(layoutData);

    std::string jsonGameName = jsonData["game"];
    if (jsonGameName != Name())
    {
        allErrors += "Layout file was a different game (" + jsonGameName + "), may not load correctly.";
    }

    for (const auto &jsonWindow : jsonData["windows"].items())
    {
        auto modeIter = std::find_if(allModes.begin(), allModes.end(), [&](const auto &mode) { return jsonWindow.key() == mode.Name; });
        if (modeIter == allModes.end())
        {
            allErrors += std::string("Layout data has unknown game mode: ") + jsonWindow.key() + "\n";
            continue;
        }

        UpdatableGameWindow* gameWindow = modeIter->Creator();

        try
        {
            std::unordered_map<std::string, std::string> windowData = jsonWindow.value();
            gameWindow->RestoreLayout(windowData);
        }
        catch (std::exception &ex)
        {
            allErrors += std::string() + "Error restoring layout for '" + jsonWindow.key() + "': " + ex.what() + "\n";
        }
    }

    for (const auto &jsonOption: jsonData["options"].items())
    {
        auto optionIter = std::find_if(allOptions.begin(), allOptions.end(), [&](const auto &option) { return jsonOption.key() == option.Name; });
        if (optionIter == allOptions.end())
        {
            allErrors += std::string("Option data has unknown option: ") + jsonOption.key() + "\n";
        }

        optionIter->Enabled = jsonOption.value();
        optionIter->OptionChanged(optionIter->Enabled);
    }

    if (!allErrors.empty())
        throw std::runtime_error(allErrors);
}
