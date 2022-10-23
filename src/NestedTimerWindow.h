#pragma once

#include <chrono>
#include <memory>
#include <functional>

#include "DraggableQWidget.h"

#include <QLCDNumber>
#include <QMenu>
#include <QGridLayout>

class NestedTimerWindow: public DraggableQWidget
{
public:
    NestedTimerWindow();

    void RefreshState();

    void AddNestedTimer(const std::string &name);
    void SetActiveTimer(const std::string &name);
    void StopAllTimers();
    void ResetAllTimers();

    std::function<void()> OnRefresh;

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    QAction *actionExit = nullptr;
    QMenu *contextMenu = nullptr;

    std::chrono::steady_clock::time_point totalTimerStart;
    std::chrono::steady_clock::time_point totalTimerEnd;
    bool isTimerActivated = false;

    QLCDNumber *totalNumberDisplay = nullptr;

    QGridLayout *nestedTimersLayout = nullptr;

    struct NestedTimer
    {
        std::string Name;
        bool Activated = false;
        bool Touched = false;
        std::chrono::steady_clock::time_point End;

        QLCDNumber *NumberDisplay = nullptr;
    };

    std::vector<NestedTimer> nestedTimers;
};
