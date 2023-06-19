#include "SimpleTimerWindow.h"

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QObject>
#include <QStyleOption>
#include <QColorDialog>

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
}

SimpleTimerWindow::SimpleTimerWindow(bool showControls)
{
    resize(225, 50);
    setWindowTitle("Timer");
    setWindowFlags(Qt::Window | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAutoFillBackground(true);

    resizeBorder = 4;

    QAction *actionExit = new QAction("Close Timer", this);
    QObject::connect(actionExit, &QAction::triggered, [&]()
    {
        close();
    });

    QAction *actionSetBgColor = new QAction("Set Background Color", this);
    QObject::connect(actionSetBgColor, &QAction::triggered, [&]()
    {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        show();
        QColor color = QColorDialog::getColor();
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();

        if (color.isValid())
        {
            setAttribute(Qt::WA_TranslucentBackground, false);
            setAutoFillBackground(true);
            QPalette pal = palette();
            pal.setColor(QPalette::Window, color);
            setPalette(pal);
        }
    });

    QAction *actionSetBgTransparent = new QAction("Set Background Transparent", this);
    QObject::connect(actionSetBgTransparent, &QAction::triggered, [&]()
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, false);

        QPalette pal = palette();
        QColor transparentColor(200, 200, 200, 2);
        pal.setColor(QPalette::Window, transparentColor);
        setPalette(pal);
    });

    QAction *actionSetBgDefault = new QAction("Set Background Default", this);
    QObject::connect(actionSetBgDefault, &QAction::triggered, [&]()
    {
        setAttribute(Qt::WA_TranslucentBackground, false);

        setAutoFillBackground(true);
        setPalette(QApplication::style()->standardPalette());
    });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionSetBgColor);
    contextMenu->addAction(actionSetBgTransparent);
    contextMenu->addAction(actionSetBgDefault);
    contextMenu->addSeparator();
    contextMenu->addAction(actionExit);

    numberDisplay = new QLCDNumber(this);
    numberDisplay->setMouseTracking(true); // for the resize cursor change to work correctly

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(numberDisplay, 95);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    if (showControls)
    {
        buttonStart = new QPushButton(this);
        buttonStart->setText(">");
        buttonStart->setMinimumSize(20, 20);
        buttonStart->setMaximumSize(50, 50);
        QObject::connect(buttonStart, &QPushButton::clicked, [&]()
        {
            StartTimer();

            buttonStop->setText("||");
            buttonStop->setEnabled(true);
            buttonStart->setEnabled(false);
        });

        buttonStop = new QPushButton(this);
        buttonStop->setText("||");
        buttonStop->setEnabled(false);
        buttonStop->setMinimumSize(20, 20);
        buttonStop->setMaximumSize(50, 50);
        QObject::connect(buttonStop, &QPushButton::clicked, [&]()
        {
            if (timerPaused)
            {
                ResetTimer();

                buttonStop->setText("||");
                buttonStop->setEnabled(false);
            }
            else
            {
                StopTimer();

                buttonStop->setText("X");
            }
            
            buttonStart->setEnabled(true);
        });

        QVBoxLayout *controlsLayout = new QVBoxLayout();
        controlsLayout->addWidget(buttonStart);
        controlsLayout->addWidget(buttonStop);

        controlsLayout->setContentsMargins(0, 0, 2, 2);
        controlsLayout->setSpacing(1);

        mainLayout->addLayout(controlsLayout, 5);
    }

    setLayout(mainLayout);
    show();

    timerStart = timerEnd = std::chrono::steady_clock::now();
    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, [this]() { timerUpdate(); });
    timer->start(1000 / 60);
}

void SimpleTimerWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        contextMenu->popup(event->globalPos());
    }
    else
        DraggableQWidget::mousePressEvent(event);
}

void SimpleTimerWindow::timerUpdate()
{
    if (timerStarted && !timerPaused)
        timerEnd = std::chrono::steady_clock::now();

    int64_t totalMsSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart).count();
    int msDigit = totalMsSinceStart % 1000;
    int secondsDigit = (totalMsSinceStart / 1000) % 60;
    int minutesDigit = (totalMsSinceStart / 1000 / 60) % 60;
    int hoursDigit = (totalMsSinceStart / 1000 / 60 / 60) % 60;

    std::string displayString = FormatDigits(hoursDigit, 1) + ":" + FormatDigits(minutesDigit, 2) + ":" + FormatDigits(secondsDigit, 2) + "." + FormatDigits(msDigit / 100, 1);
    numberDisplay->setDigitCount((int)displayString.size());
    numberDisplay->display(QString::fromStdString(displayString));
}

void SimpleTimerWindow::StartTimer()
{
    if (!timerStarted)
        timerStart = timerEnd = std::chrono::steady_clock::now();

    timerStarted = true;
    timerPaused = false;
}

void SimpleTimerWindow::StopTimer()
{
    timerPaused = true;
}

void SimpleTimerWindow::ResetTimer()
{
    timerStarted = false;
    timerPaused = true;
    timerStart = timerEnd = std::chrono::steady_clock::now();
}

void SimpleTimerWindow::RefreshState()
{
    if (shouldStartCallback && shouldStartCallback())
        StartTimer();

    if (shouldStopCallback && shouldStopCallback())
        StopTimer();

    if (shouldResetCallback && shouldResetCallback())
        ResetTimer();

    //NOTE: SimpleTimerWindow has its own timer callback for UI updates, since it might be used in manual mode without a game watcher
}
