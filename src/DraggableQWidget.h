#pragma once

#include "ClosableQWidget.h"

class DraggableQWidget: public ClosableQWidget
{
protected:
    inline void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            lastMousePos = event->globalPos();
            isDragging = true;
        }
        else
            ClosableQWidget::mousePressEvent(event);
    }

    inline void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
            isDragging = false;
        else
            ClosableQWidget::mouseReleaseEvent(event);
    }

    inline void mouseMoveEvent(QMouseEvent *event) override
    {
        if (isDragging)
        {
            QPoint diff = event->globalPos() - lastMousePos;
            move(x() + diff.x(), y() + diff.y());
            lastMousePos = event->globalPos();
        }
        else
            ClosableQWidget::mouseMoveEvent(event);
    }

private:
    QPoint lastMousePos;
    bool isDragging = false;
};
