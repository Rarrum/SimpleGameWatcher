#pragma once

#include <memory>

#include <QMenu>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidget>

#include "../GameWatcher.h"
#include "ClosableQWidget.h"
#include "UpdatableGameWindow.h"

class DebugGameStateWindow: public ClosableQWidget, public UpdatableGameWindow
{
public:
    DebugGameStateWindow(std::shared_ptr<GameWatcher> gameWatcher);

    void RefreshState() override;
    inline bool IsStillOpen() const override { return isVisible(); }

    // debug window can't be saved/restored
    inline std::unordered_map<std::string, std::string> SaveLayout() const override { return {}; }
    inline void RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData) override {}

private:
    QTableWidget *table;
    std::shared_ptr<GameWatcher> watcher;
};
