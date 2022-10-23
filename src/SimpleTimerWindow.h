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

class SimpleTimerWindow: public DraggableQWidget
{
public:
    SimpleTimerWindow(bool showControls);

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

    QTimer *timer = nullptr;
    std::chrono::steady_clock::time_point timerStart;
    std::chrono::steady_clock::time_point timerEnd;
    bool timerStarted = false;
    bool timerPaused = false;

    std::function<bool()> shouldStartCallback;
    std::function<bool()> shouldStopCallback;
    std::function<bool()> shouldResetCallback;
};
