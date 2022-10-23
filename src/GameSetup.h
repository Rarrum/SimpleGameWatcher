#pragma once

#include <vector>
#include <list>
#include <functional>
#include <string>
#include <memory>

#include <QWidget>
#include <QTimer>

#include "GameWatcher.h"
#include "SimpleTimerWindow.h"
#include "DebugGameStateWindow.h"

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

    void CreateDebugWindow(); //called by main for every game type

protected:
    virtual std::shared_ptr<GameWatcher> CreateGameSpecificWatcher() = 0;

    std::shared_ptr<GameWatcher> GetOrCreateWatcherAndStartPolling()
    {
        std::shared_ptr<GameWatcher> watcher = watcherToPoll.lock();
        if (!watcher)
        {
            watcher = CreateGameSpecificWatcher();
            watcherToPoll = watcher;
        }
        
        if (!mainPollTimer)
        {
            mainPollTimer = std::make_unique<QTimer>();
            QObject::connect(mainPollTimer.get(), &QTimer::timeout, [this](){ onWatcherTimerUpdate(); });
            mainPollTimer->start(1000 / 60);
        }

        return watcher;
    }

    SimpleTimerWindow& CreateSimpleTimer();

    virtual std::shared_ptr<GameWatcher> onWatcherTimerUpdate();

private:
    std::unique_ptr<QTimer> mainPollTimer;
    std::weak_ptr<GameWatcher> watcherToPoll;

    std::list<std::unique_ptr<DebugGameStateWindow>> allDebugWindows;
    std::list<std::unique_ptr<SimpleTimerWindow>> allSimpleTimers;
};
