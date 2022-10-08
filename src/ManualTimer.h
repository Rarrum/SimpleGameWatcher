#include <list>
#include <chrono>

#include "DraggableQWidget.h"
#include <QLCDNumber>
#include <QMenu>
#include <QTimer>
#include <QPushButton>

class ManualTimer: public DraggableQWidget
{
public:
    ManualTimer();

    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void timerUpdate();

    std::unique_ptr<QAction> actionExit;
    std::unique_ptr<QMenu> contextMenu;

    std::unique_ptr<QLCDNumber> numberDisplay;

    std::unique_ptr<QPushButton> buttonStart;
    std::unique_ptr<QPushButton> buttonStop;

    QTimer timer;
    std::chrono::steady_clock::time_point timerStart;
    std::chrono::steady_clock::time_point timerEnd;
    bool timerStarted = false;
    bool timerPaused = false;
};

extern std::list<ManualTimer> AllManualTimers;
