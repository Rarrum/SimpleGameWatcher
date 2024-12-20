#pragma once

#include <chrono>
#include <memory>
#include <functional>

#include "DraggableQWidget.h"
#include "UpdatableGameWindow.h"
#include "ScaleableLabel.h"
#include "ColorChangerWidgetHelper.h"

#include <QLCDNumber>
#include <QMenu>
#include <QGridLayout>

class NestedTimerWindow: public DraggableQWidget, public UpdatableGameWindow, public ColorChangerWidgetHelper
{
public:
    NestedTimerWindow();

    void RefreshState() override;
    inline bool IsStillOpen() const override { return isVisible(); }

    void AddNestedTimer(const std::string &name);
    void SetActiveTimer(const std::string &name);
    void SetFocusTimer(const std::string &name);
    void SetNameDisplayPrefix(const std::string &name, const std::string &prefix); // special behaviour: empty name implies all timers
    void StopAllTimers();
    void ResetAllTimers();

    std::function<void()> OnRefresh;

    std::unordered_map<std::string, std::string> SaveLayout() const override;
    void RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

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
        std::string DisplayPrefix;
        bool Activated = false;
        bool Focused = false;
        bool Touched = false;
        std::chrono::steady_clock::time_point End;

        ScaleableLabel *Label = nullptr;
        QLCDNumber *NumberDisplay = nullptr;
    };

    std::vector<NestedTimer> nestedTimers;
};
