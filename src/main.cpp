#include <vector>
#include <memory>

#include <QApplication>
#include <QPushButton>
#include <QObject>

#include "ClosableQWidget.h"
#include "ManualTimer.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    std::unique_ptr<ClosableQWidget> window = std::make_unique<ClosableQWidget>();
    window->resize(320, 240);

    QPushButton button(window.get());
    button.setText("Create Manual Timer");
    QObject::connect(&button, &QPushButton::clicked, [&]()
    {
        AllManualTimers.emplace_back();
    });

    window->setWindowTitle("EasyAutoTracker");
    window->show();

    window->closeCallback = [&]()
    {
        AllManualTimers.clear();
    };

    return app.exec();
}
