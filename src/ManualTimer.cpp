#include "ManualTimer.h"

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QObject>

std::list<ManualTimer> AllManualTimers;

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

ManualTimer::ManualTimer()
{
    resize(225, 50);
    setWindowTitle("Manual Timer");
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    closeCallback = [&]()
    {
        AllManualTimers.remove_if([&](const auto &item) { return this == &item; });
    };

    actionExit = std::make_unique<QAction>("Close Timer", this);
    QObject::connect(actionExit.get(), &QAction::triggered, [&]()
    {
        close();
    });

    contextMenu = std::make_unique<QMenu>(this);
    contextMenu->addAction(actionExit.get());

    numberDisplay = std::make_unique<QLCDNumber>(this);
    numberDisplay->resize(200, 50);

    timerStart = timerEnd = std::chrono::steady_clock::now();
    QObject::connect(&timer, &QTimer::timeout, [this]() { timerUpdate(); });
    timer.start(1000 / 60);

    buttonStart = std::make_unique<QPushButton>(this);
    buttonStart->setText(">");
    buttonStart->setGeometry(200, 0, 25, 25);
    QObject::connect(buttonStart.get(), &QPushButton::clicked, [&]()
    {
        if (!timerStarted)
            timerStart = timerEnd = std::chrono::steady_clock::now();

        timerStarted = true;
        timerPaused = false;
        buttonStop->setText("||");
    });

    buttonStop = std::make_unique<QPushButton>(this);
    buttonStop->setText("||");
    buttonStop->setGeometry(200, 25, 25, 25);
    QObject::connect(buttonStop.get(), &QPushButton::clicked, [&]()
    {
        if (timerPaused)
        {
            buttonStop->setText("||");
            timerStarted = false;
            timerStart = timerEnd = std::chrono::steady_clock::now();
        }
        else
        {
            buttonStop->setText("X");
            timerPaused = true;
        }
    });

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
