#pragma once

#include <memory>

#include <QMenu>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidget>

#include "GameWatcher.h"
#include "ClosableQWidget.h"

class DebugGameStateWindow: public ClosableQWidget
{
public:
    DebugGameStateWindow(std::shared_ptr<GameWatcher> gameWatcher);
    inline void ClearWatcher() { watcher.reset(); } //TODO: why do we need this?  dtor should free watcher.. is dtor not being called on window close!?

    void RefreshStateFromWatcher();

private:
    QTableWidget *table;
    std::shared_ptr<GameWatcher> watcher;
};
