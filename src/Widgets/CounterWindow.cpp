#include "CounterWindow.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

CounterWindow::CounterWindow(bool createDefaultCounterWithControls)
{
    setWindowTitle("Counter");
    setWindowFlags(Qt::Window | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    SetupColorChanger(this);

    resizeBorder = 4;

    setContentsMargins(1, 1, 1, 1);

    actionExit = new QAction("Close Counter", this);
    QObject::connect(actionExit, &QAction::triggered, [&]()
    {
        close();
    });

    contextMenu = new QMenu(this);
    AddColorOptionsToMenu(contextMenu, true, true);
    contextMenu->addSeparator();
    contextMenu->addAction(actionExit);

    nestedCountersLayout = new QGridLayout();
    nestedCountersLayout->setVerticalSpacing(1);

    QWidget *nestedCountersLayoutHolder = new QWidget();
    nestedCountersLayoutHolder->setLayout(nestedCountersLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(1, 1, 1, 1); // gets our controls closer to the edge - TODO: investigate run-time setGeometry warning caused by this
    mainLayout->addWidget(nestedCountersLayoutHolder);
    setLayout(mainLayout);

    resize(200, 35);

    if (createDefaultCounterWithControls)
    {
        AddCounter("default");

        buttonPlus = new QPushButton(this);
        buttonPlus->setText("+");
        buttonPlus->setMinimumSize(20, 20);
        buttonPlus->setMaximumSize(50, 50);
        QObject::connect(buttonPlus, &QPushButton::clicked, [&]()
        {
            AdjustCounter("default", 1);
        });

        buttonMinus = new QPushButton(this);
        buttonMinus->setText("-");
        buttonMinus->setMinimumSize(20, 20);
        buttonMinus->setMaximumSize(50, 50);
        QObject::connect(buttonMinus, &QPushButton::clicked, [&]()
        {
            AdjustCounter("default", -1);
        });

        QVBoxLayout *controlsLayout = new QVBoxLayout();
        controlsLayout->addWidget(buttonPlus);
        controlsLayout->addWidget(buttonMinus);

        controlsLayout->setContentsMargins(0, 0, 2, 2);
        controlsLayout->setSpacing(1);

        mainLayout->addLayout(controlsLayout, 5);

        // in manual mode we aren't hooked up to game state, so run our own timer for refreshes
        QTimer *timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, [this]() { RefreshState(); });
        timer->start(1000 / 60);
    }

    // setMouseTracking must be explicitly set on all children for it to actually work as expected
    for (QWidget *child : findChildren<QWidget*>())
        child->setMouseTracking(true);

    ResetAllCounters();
    show();
}

void CounterWindow::RefreshState()
{
    if (OnRefresh)
        OnRefresh();

    for (NestedCounter &nested : nestedCounters)
    {
        std::string displayValue = std::to_string(nested.Value);
        int digits = 3;
        if ((int)displayValue.size() > digits)
            digits = (int)displayValue.size();

        nested.NumberDisplay->setDigitCount(digits);
        nested.NumberDisplay->display(QString::fromStdString(displayValue));
    }
}

void CounterWindow::AddCounter(const std::string &name)
{
    resize(width(), height() + 35);

    int row = (int)nestedCounters.size();

    NestedCounter &nested = nestedCounters.emplace_back();
    nested.Name = name;

    nested.NumberDisplay = new QLCDNumber();
    nested.NumberDisplay->setMouseTracking(true); // for the resize cursor change to work correctly
    nested.NumberDisplay->setFrameStyle(QFrame::NoFrame);
    nested.NumberDisplay->setDigitCount(3);
    nested.NumberDisplay->display("0");

    QSizePolicy sizePolicy = nested.NumberDisplay->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    nested.NumberDisplay->setSizePolicy(sizePolicy);

    nested.Label = new ScaleableLabel(name);
    nested.Label->setMouseTracking(true); // for the resize cursor change to work correctly
    nested.Label->setBuddy(nested.NumberDisplay);
    nested.Label->FillFactor = 0.8f;

    //nested.Label->setFrameShape(QFrame::Box); // useful for debugging layout

    if (name != "default")
        nestedCountersLayout->addWidget(nested.Label, row, 0);

    nestedCountersLayout->addWidget(nested.NumberDisplay, row, 1);
    nestedCountersLayout->setRowStretch(row, 1);
}

void CounterWindow::AdjustCounter(const std::string &name, int64_t changeAmount)
{
    for (NestedCounter &nested : nestedCounters)
    {
        if (nested.Name == name)
            nested.Value += changeAmount;
    }
}

void CounterWindow::SetCounter(const std::string &name, int64_t value)
{
    for (NestedCounter &nested : nestedCounters)
    {
        if (nested.Name == name)
            nested.Value = value;
    }
}

void CounterWindow::ResetAllCounters()
{
    for (NestedCounter &nested : nestedCounters)
    {
        nested.Value = 0;
    }
}

std::unordered_map<std::string, std::string> CounterWindow::SaveLayout() const
{
    std::unordered_map<std::string, std::string> layoutData;
    DraggableQWidget::SaveLayoutIn(layoutData);
    ColorChangerWidgetHelper::SaveLayoutIn(layoutData);
    return layoutData;
}

void CounterWindow::RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData)
{
    DraggableQWidget::RestoreLayoutFrom(layoutData);
    ColorChangerWidgetHelper::RestoreLayoutFrom(layoutData);
}

void CounterWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        contextMenu->popup(event->globalPos());
    }
    else
        DraggableQWidget::mousePressEvent(event);
}

void CounterWindow::resizeEvent(QResizeEvent *event)
{
    setUpdatesEnabled(false);

    DraggableQWidget::resizeEvent(event);

    if (!nestedCounters.empty())
    {
        // Resize all counters to take up the same space
        QRect layoutRect = nestedCountersLayout->geometry();
        int heightPerRow = layoutRect.height() / ((int)nestedCounters.size() + 1);
        int minColWidth = layoutRect.width() / 2 - 11 * 2;

        if (heightPerRow > 2)
        {
            for (NestedCounter &nested : nestedCounters)
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
