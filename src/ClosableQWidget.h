#pragma once

#include <functional>

#include <QWidget>
#include <QtGui/QCloseEvent>

class ClosableQWidget: public QWidget
{
public:
    std::function<void()> closeCallback;

protected:
    inline void closeEvent(QCloseEvent *event) override
    {
        if (closeCallback)
            closeCallback();

        QWidget::closeEvent(event);
    }
};
