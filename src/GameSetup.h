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
#include "Widgets/SimpleTimerWindow.h"
#include "Widgets/NestedTimerWindow.h"
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
    virtual std::vector<GameSetupMode>& Entries() = 0;

    virtual void StartWatching();
    std::function<void()> OnWatcherUpdate;
    virtual void CloseWindowsAndStopWatching();

    inline std::shared_ptr<GameWatcher> Watcher() { return watcherToPoll; }

    void CreateDebugWindow(); //called by main as needed

protected:
    virtual std::shared_ptr<GameWatcher> CreateGameSpecificWatcher() = 0;

    virtual void onWatcherTimerUpdate();

    SimpleTimerWindow* CreateSimpleTimer();
    NestedTimerWindow* CreateNestedTimer();

private:
    std::unique_ptr<QTimer> mainPollTimer;
    std::shared_ptr<GameWatcher> watcherToPoll;

    std::list<std::unique_ptr<DebugGameStateWindow>> allDebugWindows;
    std::list<std::unique_ptr<UpdatableGameWindow>> allNormalWindows;
};
