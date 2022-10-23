#include "NestedTimerWindow.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

namespace
{
    //TODO: switch back to std::format after clang/etc support it in LTS releases
    std::string FormatDigits(int64_t number, int zeroPadToLength)
    {
        std::string formatted = std::to_string(number);
        while (formatted.size() < zeroPadToLength)
            formatted = "0" + formatted;

        return formatted;
    }

    void SetTimerDisplayValue(QLCDNumber *numberDisplay, int64_t totalMilliseconds, bool showMilliseconds)
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
    }
}

NestedTimerWindow::NestedTimerWindow()
{
    setWindowTitle("Timer");
    setWindowFlags(Qt::Window | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    resizeBorder = 4;

    actionExit = new QAction("Close Timer", this);
    QObject::connect(actionExit, &QAction::triggered, [&]()
    {
        close();
    });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionExit);

    totalNumberDisplay = new QLCDNumber();
    totalNumberDisplay->setMouseTracking(true); // for the resize cursor change to work correctly
    SetTimerDisplayValue(totalNumberDisplay, 0, true);
    totalNumberDisplay->setMinimumHeight(25);

    nestedTimersLayout = new QGridLayout();

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    QWidget *nestedTimersLayoutHolder = new QWidget();
    nestedTimersLayoutHolder->setLayout(nestedTimersLayout);
    splitter->addWidget(nestedTimersLayoutHolder);
    splitter->addWidget(totalNumberDisplay);
    splitter->setStretchFactor(0, 90);
    splitter->setStretchFactor(1, 10);

    QVBoxLayout *dummyMainLayout = new QVBoxLayout();
    dummyMainLayout->addWidget(splitter);
    setLayout(dummyMainLayout);

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
    SetTimerDisplayValue(totalNumberDisplay, totalMsSinceStart, true);

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

    QLabel *label = new QLabel(QString::fromStdString(name));
    label->setBuddy(nested.NumberDisplay);

    nestedTimersLayout->addWidget(label, row, 0);
    nestedTimersLayout->addWidget(nested.NumberDisplay, row, 1);
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
            nested.Activated = false;
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

    for (NestedTimer &nested : nestedTimers)
    {
        nested.End = now;
        nested.Activated = false;
        nested.Touched = false;
    }
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
