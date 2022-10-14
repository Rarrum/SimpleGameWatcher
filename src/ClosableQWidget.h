#pragma once

#include <functional>

#include <QWidget>
#include <QtGui/QCloseEvent>

class ClosableQWidget: public QWidget
{
public:
    inline ClosableQWidget(QWidget *parent = nullptr): QWidget(parent) {}

    std::function<void(ClosableQWidget&)> closeCallback;

protected:
    inline void closeEvent(QCloseEvent *event) override
    {
        if (closeCallback)
            closeCallback(*this);

        QWidget::closeEvent(event);
    }
};
