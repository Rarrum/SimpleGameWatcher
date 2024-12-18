#include "NestedTimerWindow.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

#include "ScaleableLabel.h"

namespace
{
    //TODO: switch back to std::format after clang/etc support it in LTS releases
    std::string FormatDigits(int64_t number, int zeroPadToLength)
    {
        std::string formatted = std::to_string(number);
        while ((int)formatted.size() < zeroPadToLength)
            formatted = "0" + formatted;

        return formatted;
    }

    void SetTimerDisplayValue(QLCDNumber *numberDisplay, int64_t totalMilliseconds, bool showMilliseconds, bool alwaysShow = false)
    {
        if (totalMilliseconds < 0)
            totalMilliseconds = 0;

        int msDigit = totalMilliseconds % 1000;
        int secondsDigit = (totalMilliseconds / 1000) % 60;
        int minutesDigit = (totalMilliseconds / 1000 / 60) % 60;
        int hoursDigit = (totalMilliseconds / 1000 / 60 / 60) % 60;

        std::string displayString = FormatDigits(hoursDigit, 1) + ":" + FormatDigits(minutesDigit, 2) + ":" + FormatDigits(secondsDigit, 2);
        if (showMilliseconds)
            displayString += "." + FormatDigits(msDigit / 100, 1);

        numberDisplay->setDigitCount((int)displayString.size());

        numberDisplay->display(QString::fromStdString(displayString));
        numberDisplay->setVisible(alwaysShow || totalMilliseconds != 0);
    }
}

NestedTimerWindow::NestedTimerWindow()
{
    setWindowTitle("Timer");
    setWindowFlags(Qt::Window | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    SetupColorChanger(this);

    resizeBorder = 4;

    setContentsMargins(1, 1, 1, 1);

    actionExit = new QAction("Close Timer", this);
    QObject::connect(actionExit, &QAction::triggered, [&]()
    {
        close();
    });

    contextMenu = new QMenu(this);
    AddColorOptionsToMenu(contextMenu, true, true);
    contextMenu->addSeparator();
    contextMenu->addAction(actionExit);

    totalNumberDisplay = new QLCDNumber();
    SetTimerDisplayValue(totalNumberDisplay, 0, true, true);
    totalNumberDisplay->setMinimumHeight(25);

    nestedTimersLayout = new QGridLayout();
    nestedTimersLayout->setVerticalSpacing(1);

    QWidget *nestedTimersLayoutHolder = new QWidget();
    nestedTimersLayoutHolder->setLayout(nestedTimersLayout);

    QSplitter *bottomSplitter = new QSplitter(Qt::Vertical);
    bottomSplitter->addWidget(nestedTimersLayoutHolder);
    bottomSplitter->addWidget(totalNumberDisplay);
    bottomSplitter->setStretchFactor(0, 88);
    bottomSplitter->setStretchFactor(1, 12);

    QVBoxLayout *dummyMainLayout = new QVBoxLayout();
    dummyMainLayout->setContentsMargins(1, 1, 1, 1); // gets our controls closer to the edge - TODO: investigate run-time setGeometry warning caused by this
    dummyMainLayout->addWidget(bottomSplitter);
    setLayout(dummyMainLayout);

    resize(200, 200);

    // setMouseTracking must be explicitly set on all children for it to actually work as expected
    for (QWidget *child : findChildren<QWidget*>())
        child->setMouseTracking(true);

    ResetAllTimers();
    show();
}

void NestedTimerWindow::RefreshState()
{
    if (OnRefresh)
        OnRefresh();

    auto now = std::chrono::steady_clock::now();
    if (isTimerActivated)
    {
        totalTimerEnd = now;

        for (NestedTimer &nested : nestedTimers)
        {
            if (nested.Activated)
                nested.End = now;
        }
    }

    int64_t totalMsSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimerEnd - totalTimerStart).count();
    SetTimerDisplayValue(totalNumberDisplay, totalMsSinceStart, true, true);

    for (NestedTimer &nested : nestedTimers)
    {
        if (nested.Touched)
        {
            nested.NumberDisplay->setVisible(true);
            int64_t msForNested = std::chrono::duration_cast<std::chrono::milliseconds>(nested.End - totalTimerStart).count();
            SetTimerDisplayValue(nested.NumberDisplay, msForNested, false);
        }
        else
        {
            nested.NumberDisplay->setVisible(false);
        }
    }
}

void NestedTimerWindow::AddNestedTimer(const std::string &name)
{
    resize(width(), height() + 40);

    int row = (int)nestedTimers.size();

    NestedTimer &nested = nestedTimers.emplace_back();
    nested.Name = name;

    nested.NumberDisplay = new QLCDNumber();
    nested.NumberDisplay->setMouseTracking(true); // for the resize cursor change to work correctly
    nested.NumberDisplay->setFrameStyle(QFrame::NoFrame);
    SetTimerDisplayValue(nested.NumberDisplay, 0, false);

    QSizePolicy sizePolicy = nested.NumberDisplay->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    nested.NumberDisplay->setSizePolicy(sizePolicy);
    nested.NumberDisplay->setVisible(false);

    nested.Label = new ScaleableLabel(name);
    nested.Label->setMouseTracking(true); // for the resize cursor change to work correctly
    nested.Label->setBuddy(nested.NumberDisplay);
    nested.Label->FillFactor = 0.8f;

    //nested.Label->setFrameShape(QFrame::Box); // useful for debugging layout

    nestedTimersLayout->addWidget(nested.Label, row, 0);
    nestedTimersLayout->addWidget(nested.NumberDisplay, row, 1);
    nestedTimersLayout->setRowStretch(row, 1);
}

void NestedTimerWindow::SetActiveTimer(const std::string &name)
{
    if (!isTimerActivated)
    {
        isTimerActivated = true;
        totalTimerStart = totalTimerEnd = std::chrono::steady_clock::now();
    }

    for (NestedTimer &nested : nestedTimers)
    {
        if (nested.Name == name)
        {
            nested.Activated = true;
            nested.Touched = true;
        }
        else
        {
            nested.Activated = false;
        }
    }
}

void NestedTimerWindow::SetFocusTimer(const std::string &name)
{
    for (NestedTimer &nested : nestedTimers)
    {
        if (nested.Name == name)
        {
            if (!nested.Focused)
            {
                QFont font = nested.Label->font();
                font.setWeight(QFont::Weight::Bold);
                nested.Label->ChangeAndScaleFont(font);
            }

            nested.Focused = true;
        }
        else
        {
            if (nested.Focused)
            {
                QFont font = nested.Label->font();
                font.setWeight(QFont::Weight::Normal);
                nested.Label->ChangeAndScaleFont(font);
            }

            nested.Focused = false;
        }
    }
}

void NestedTimerWindow::SetNameDisplayPrefix(const std::string &name, const std::string &prefix)
{
    for (NestedTimer &nested : nestedTimers)
    {
        if (nested.Name == name)
        {
            nested.DisplayPrefix = prefix;
            nested.Label->setText(QString::fromStdString(nested.DisplayPrefix + nested.Name));
            break;
        }
    }
}

void NestedTimerWindow::StopAllTimers()
{
    if (!isTimerActivated)
        return;

    auto now = std::chrono::steady_clock::now();
    totalTimerEnd = now;
    isTimerActivated = false;

    for (NestedTimer &nested : nestedTimers)
    {
        if (nested.Activated)
            nested.End = now;

        nested.Activated = false;
    }
}

void NestedTimerWindow::ResetAllTimers()
{
    auto now = std::chrono::steady_clock::now();
    isTimerActivated = false;
    totalTimerStart = totalTimerEnd = now;

    SetFocusTimer(""); // clear's currently bolded timer

    for (NestedTimer &nested : nestedTimers)
    {
        nested.End = now;
        nested.Activated = false;
        nested.Focused = false;
        nested.Touched = false;
    }
}

std::unordered_map<std::string, std::string> NestedTimerWindow::SaveLayout() const
{
    std::unordered_map<std::string, std::string> layoutData;
    DraggableQWidget::SaveLayoutIn(layoutData);
    ColorChangerWidgetHelper::SaveLayoutIn(layoutData);
    return layoutData;
}

void NestedTimerWindow::RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData)
{
    DraggableQWidget::RestoreLayoutFrom(layoutData);
    ColorChangerWidgetHelper::RestoreLayoutFrom(layoutData);
}

void NestedTimerWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        contextMenu->popup(event->globalPos());
    }
    else
        DraggableQWidget::mousePressEvent(event);
}

void NestedTimerWindow::resizeEvent(QResizeEvent *event)
{
    setUpdatesEnabled(false);

    DraggableQWidget::resizeEvent(event);

    if (!nestedTimers.empty())
    {
        // Resize all timers to take up the same space
        QRect layoutRect = nestedTimersLayout->geometry();
        int heightPerRow = layoutRect.height() / ((int)nestedTimers.size() + 1);
        int minColWidth = layoutRect.width() / 2 - 11 * 2;

        if (heightPerRow > 2)
        {
            for (NestedTimer &nested : nestedTimers)
            {
                nested.Label->setMaximumHeight(heightPerRow - 1);
                nested.NumberDisplay->setMaximumHeight(heightPerRow - 1);

                nested.Label->setMaximumWidth(layoutRect.width() / 2);
                if (minColWidth > 5)
                    nested.NumberDisplay->setMinimumWidth(minColWidth);
            }
        }

        //TODO: we would like the labels to all use the same font size, but we can't just change it here, since ChangeAndScaleFont will clobber it
    }

    setUpdatesEnabled(true);
}
