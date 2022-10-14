#include <list>
#include <chrono>

#include "DraggableQWidget.h"

#include <QLCDNumber>
#include <QMenu>
#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class ManualTimer: public DraggableQWidget
{
public:
    ManualTimer(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void timerUpdate();

    QAction *actionExit;
    QMenu *contextMenu;

    QLCDNumber *numberDisplay;

    QPushButton *buttonStart;
    QPushButton *buttonStop;

    QVBoxLayout *controlsLayout;
    QHBoxLayout *mainLayout;

    QTimer *timer;
    std::chrono::steady_clock::time_point timerStart;
    std::chrono::steady_clock::time_point timerEnd;
    bool timerStarted = false;
    bool timerPaused = false;
};
