#pragma once

#include <vector>
#include <list>
#include <functional>
#include <string>
#include <memory>

#include <QWidget>
#include <QTimer>

#include "GameWatcher.h"
#include "Widgets/UpdatableGameWindow.h"
#include "Widgets/DebugGameStateWindow.h"

struct GameSetupMode
{
    GameSetupMode() = default;
    inline GameSetupMode(const std::string &name, std::function<UpdatableGameWindow*()> creator)
    {
        Name = name;
        Creator = creator;
    }

    std::string Name;
    std::function<UpdatableGameWindow*()> Creator;
};

struct GameSetupOption
{
    GameSetupOption() = default;
    inline GameSetupOption(const std::string &name, std::function<void(bool enabled)> optionChanged)
    {
        Name = name;
        OptionChanged = optionChanged;
    }

    std::string Name;
    bool Enabled = false;
    std::function<void(bool enabled)> OptionChanged;
};

class GameSetup
{
public:
    virtual ~GameSetup() = default;

    virtual std::string Name() const = 0;
    const std::vector<GameSetupMode>& GameModes();
    std::vector<GameSetupOption>& GameOptions(); // note: non-const because the enabled state can mutate with user input

    virtual void StartWatching();
    std::function<void()> OnWatcherUpdate;
    virtual void CloseWindowsAndStopWatching();

    inline std::shared_ptr<GameWatcher> Watcher() { return watcherToPoll; }

    void CreateDebugWindow(); //called by main as needed

    std::string SaveLayout() const;
    void RestoreLayout(const std::string &layoutData);

protected:
    virtual std::shared_ptr<GameWatcher> CreateGameSpecificWatcher() = 0;

    virtual void OnWatcherTimerUpdate();

    void AddGameMode(const std::string &name, std::function<std::unique_ptr<UpdatableGameWindow>()> creator);
    void AddGameBoolOption(const std::string &name, std::function<void(bool enabled)> optionChanged, bool initialState);

private:
    std::unique_ptr<QTimer> mainPollTimer;
    std::shared_ptr<GameWatcher> watcherToPoll;

    std::vector<GameSetupMode> allModes;
    std::vector<GameSetupOption> allOptions;

    struct NormalWindow
    {
        inline NormalWindow(const std::string gameMode, std::unique_ptr<UpdatableGameWindow> window)
        {
            GameMode =  gameMode;
            Window = std::move(window);
        }

        std::string GameMode;
        std::unique_ptr<UpdatableGameWindow> Window;
    };

    std::list<std::unique_ptr<DebugGameStateWindow>> allDebugWindows;
    std::list<NormalWindow> allNormalWindows;
};
