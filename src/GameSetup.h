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
    inline GameSetupMode(const std::string &name, std::function<void()> creator)
    {
        Name = name;
        Creator = creator;
    }

    std::string Name;
    std::function<void()> Creator;
};

class GameSetup
{
public:
    virtual ~GameSetup() = default;

    virtual std::string Name() const = 0;
    virtual const std::vector<GameSetupMode>& Entries();

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

    void AddGameMode(const std::string name, std::function<std::unique_ptr<UpdatableGameWindow>()> creator);

private:
    std::unique_ptr<QTimer> mainPollTimer;
    std::shared_ptr<GameWatcher> watcherToPoll;

    std::vector<GameSetupMode> allModes;

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
