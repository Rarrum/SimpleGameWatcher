#pragma once

#include <chrono>
#include <memory>

#include "DraggableQWidget.h"
#include "UpdatableGameWindow.h"
#include "ColorChangerWidgetHelper.h"

#include <QLCDNumber>
#include <QMenu>
#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SimpleTimerWindow: public DraggableQWidget, public UpdatableGameWindow, public ColorChangerWidgetHelper
{
public:
    SimpleTimerWindow(bool showControls = false);

    inline void SetStartCheck(std::function<bool()> shouldStart) { shouldStartCallback = shouldStart; }
    inline void SetStopCheck(std::function<bool()> shouldStop) { shouldStopCallback = shouldStop; }
    inline void SetResetCheck(std::function<bool()> shouldReset) { shouldResetCallback = shouldReset; }
    void SetCurrentTime(uint64_t totalMilliseconds);

    std::function<void()> OnRefresh;

    void RefreshState() override;
    inline bool IsStillOpen() const override { return isVisible(); }

    std::unordered_map<std::string, std::string> SaveLayout() const override;
    void RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData) override;

protected:
    void StartTimer();
    void StopTimer();
    void ResetTimer();

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void timerUpdate();

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
