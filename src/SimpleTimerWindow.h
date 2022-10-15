#pragma once

#include <chrono>

#include "DraggableQWidget.h"

#include <QLCDNumber>
#include <QMenu>
#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class SimpleTimerWindow: public DraggableQWidget
{
public:
    SimpleTimerWindow(bool showControls, QWidget *parent);

    void mousePressEvent(QMouseEvent *event) override;

    void StartTimer();
    void StopTimer();
    void ResetTimer();

private slots:
    void timerUpdate();

    QAction *actionExit = nullptr;
    QMenu *contextMenu = nullptr;

    QLCDNumber *numberDisplay = nullptr;

    QPushButton *buttonStart = nullptr;
    QPushButton *buttonStop = nullptr;

    QVBoxLayout *controlsLayout = nullptr;
    QHBoxLayout *mainLayout = nullptr;

    QTimer *timer = nullptr;
    std::chrono::steady_clock::time_point timerStart;
    std::chrono::steady_clock::time_point timerEnd;
    bool timerStarted = false;
    bool timerPaused = false;
};
