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

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    std::unique_ptr<ClosableQWidget> window = std::make_unique<ClosableQWidget>();
    window->resize(500, 400);

    QHBoxLayout topLayout;

    QGroupBox manualBox("Manual Controlled");
    QGroupBox autoBox("Auto Controlled");
    topLayout.addWidget(&manualBox);
    topLayout.addWidget(&autoBox);

    QVBoxLayout manualLayout;
    manualLayout.setAlignment(Qt::AlignTop);
    QVBoxLayout autoLayout;
    autoLayout.setAlignment(Qt::AlignTop);

    QPushButton button;
    button.setText("Create Simple Timer");
    QObject::connect(&button, &QPushButton::clicked, [&]()
    {
        AllManualTimers.emplace_back();
    });
    manualLayout.addWidget(&button);

    manualBox.setLayout(&manualLayout);
    autoBox.setLayout(&autoLayout);

    window->setWindowTitle("EasyAutoTracker");
    window->setLayout(&topLayout);
    window->show();

    window->closeCallback = [&]()
    {
        AllManualTimers.clear();
    };

    return app.exec();
}
