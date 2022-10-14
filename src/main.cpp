#include <vector>
#include <memory>

#include <QApplication>
#include <QPushButton>
#include <QObject>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ClosableQWidget.h"
#include "ManualTimer.h"

namespace
{
    std::list<ManualTimer*> AllManualTimers;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ClosableQWidget window = ClosableQWidget();
    window.resize(500, 400);

    QGroupBox *manualBox = new QGroupBox("Manual Controlled");
    QGroupBox *autoBox = new QGroupBox("Auto Controlled");
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(manualBox);
    topLayout->addWidget(autoBox);

    QVBoxLayout *manualLayout = new QVBoxLayout();
    manualLayout->setAlignment(Qt::AlignTop);
    QVBoxLayout *autoLayout = new QVBoxLayout();;
    autoLayout->setAlignment(Qt::AlignTop);

    QPushButton *button = new QPushButton(&window);
    button->setText("Create Simple Timer");
    QObject::connect(button, &QPushButton::clicked, [&]()
    {
        ManualTimer *newTimer = new ManualTimer(&window);
        AllManualTimers.emplace_back(newTimer);
        newTimer->closeCallback = [&](ClosableQWidget &closedTimer)
        {
            AllManualTimers.remove_if([&](const auto &item) { return &closedTimer == item; });
        };
    });
    manualLayout->addWidget(button);

    manualBox->setLayout(manualLayout);
    autoBox->setLayout(autoLayout);

    window.setWindowTitle("EasyAutoTracker");
    window.setLayout(topLayout);
    window.show();

    return app.exec();
}
