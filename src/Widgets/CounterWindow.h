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
#include <QPushButton>

class CounterWindow: public DraggableQWidget, public UpdatableGameWindow, public ColorChangerWidgetHelper
{
public:
    CounterWindow(bool createDefaultCounterWithControls = false);

    void RefreshState() override;
    inline bool IsStillOpen() const override { return isVisible(); }

    void AddCounter(const std::string &name);
    void AdjustCounter(const std::string &name, int64_t changeAmount);
    void SetCounter(const std::string &name, int64_t value);
    void ResetAllCounters();

    std::unordered_map<std::string, std::string> SaveLayout() const override;
    void RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    QAction *actionExit = nullptr;
    QMenu *contextMenu = nullptr;

    QPushButton *buttonPlus = nullptr;
    QPushButton *buttonMinus = nullptr;

    QGridLayout *nestedCountersLayout = nullptr;

    struct NestedCounter
    {
        std::string Name;
        int64_t Value = 0;

        ScaleableLabel *Label = nullptr;
        QLCDNumber *NumberDisplay = nullptr;
    };

    std::vector<NestedCounter> nestedCounters;
};
