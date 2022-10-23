#pragma once

#include <chrono>
#include <memory>

#include "DraggableQWidget.h"

#include <QLCDNumber>
#include <QMenu>
#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "GameWatcher.h"

class SimpleTimerWindow: public DraggableQWidget
{
public:
    SimpleTimerWindow(bool showControls);

    inline void SetWatcher(std::shared_ptr<GameWatcher> gameWatcher) { watcher = gameWatcher; }

    inline void SetStartCheck(std::function<bool()> shouldStart) { shouldStartCallback = shouldStart; }
    inline void SetStopCheck(std::function<bool()> shouldStop) { shouldStopCallback = shouldStop; }
    inline void SetResetCheck(std::function<bool()> shouldReset) { shouldResetCallback = shouldReset; }

    void RefreshStateFromWatcher();

protected:
    void StartTimer();
    void StopTimer();
    void ResetTimer();

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void timerUpdate();

    QAction *actionExit = nullptr;
    QMenu *contextMenu = nullptr;

    QLCDNumber *numberDisplay = nullptr;

    QPushButton *buttonStart = nullptr;
    QPushButton *buttonStop = nullptr;

    QVBoxLayout *controlsLayout = nullptr;
    QHBoxLayout *mainLayout = nullptr;

    QTimer *timer = nullptr;
    std::chrono::steady_clock::time_point timerStart;
    std::chrono::steady_clock::time_point timerEnd;
    bool timerStarted = false;
    bool timerPaused = false;

    std::shared_ptr<GameWatcher> watcher;
    std::function<bool()> shouldStartCallback;
    std::function<bool()> shouldStopCallback;
    std::function<bool()> shouldResetCallback;
};
