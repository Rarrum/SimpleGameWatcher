#include "ManualTimer.h"

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QObject>

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

ManualTimer::ManualTimer(QWidget *parent): DraggableQWidget(parent)
{
    resize(225, 50);
    setWindowTitle("Manual Timer");
    setWindowFlags(Qt::Window | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint);

    resizeBorder = 4;

    actionExit = new QAction("Close Timer", this);
    QObject::connect(actionExit, &QAction::triggered, [&]()
    {
        close();
    });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionExit);

    numberDisplay = new QLCDNumber(this);
    numberDisplay->setMouseTracking(true); // for the resize cursor change to work correctly

    timerStart = timerEnd = std::chrono::steady_clock::now();
    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, [this]() { timerUpdate(); });
    timer->start(1000 / 60);

    buttonStart = new QPushButton(this);
    buttonStart->setText(">");
    buttonStart->setMinimumSize(20, 20);
    buttonStart->setMaximumSize(50, 50);
    QObject::connect(buttonStart, &QPushButton::clicked, [&]()
    {
        if (!timerStarted)
            timerStart = timerEnd = std::chrono::steady_clock::now();

        timerStarted = true;
        timerPaused = false;
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
            buttonStop->setText("||");
            buttonStop->setEnabled(false);
            timerStarted = false;
            timerStart = timerEnd = std::chrono::steady_clock::now();
        }
        else
        {
            buttonStop->setText("X");
            timerPaused = true;
        }
        
        buttonStart->setEnabled(true);
    });

    controlsLayout = new QVBoxLayout();
    controlsLayout->setContentsMargins(0, 0, 2, 2);
    controlsLayout->setSpacing(1);
    controlsLayout->addWidget(buttonStart);
    controlsLayout->addWidget(buttonStop);

    mainLayout = new QHBoxLayout();
    mainLayout->addWidget(numberDisplay, 95);
    mainLayout->addLayout(controlsLayout, 5);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);
    show();
}

void ManualTimer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        contextMenu->popup(event->globalPos());
    }
    else
        DraggableQWidget::mousePressEvent(event);
}

void ManualTimer::timerUpdate()
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
