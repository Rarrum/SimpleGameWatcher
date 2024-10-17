#pragma once

#include "ClosableQWidget.h"

#include <unordered_map>
#include <string>

class DraggableQWidget: public ClosableQWidget
{
public:
    inline DraggableQWidget(QWidget *parent = nullptr): ClosableQWidget(parent)
    {
        setMouseTracking(true);
    }

    int resizeBorder = 0;

    inline void SaveLayoutIn(std::unordered_map<std::string, std::string> &layoutData) const
    {
        layoutData.emplace("x", std::to_string(x()));
        layoutData.emplace("y", std::to_string(y()));
        layoutData.emplace("width", std::to_string(width()));
        layoutData.emplace("height", std::to_string(height()));
    }

    inline void RestoreLayoutFrom(const std::unordered_map<std::string, std::string> &layoutData)
    {
        int xNew = std::stoi(layoutData.at("x"));
        int yNew = std::stoi(layoutData.at("y"));
        int widNew = std::stoi(layoutData.at("width"));
        int heiNew = std::stoi(layoutData.at("height"));

        setGeometry(xNew, yNew, widNew, heiNew);
    }

protected:
    inline void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            lastMousePos = event->globalPos();

            if (hoverL || hoverR || hoverT || hoverB)
                isResizing = true;
            else
                isDragging = true;
        }
        else
            ClosableQWidget::mousePressEvent(event);
    }

    inline void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            isDragging = false;
            isResizing = false;
        }
        else
            ClosableQWidget::mouseReleaseEvent(event);
    }

    inline void mouseMoveEvent(QMouseEvent *event) override
    {
        if (isDragging)
        {
            QPoint diff = event->globalPos() - lastMousePos;
            lastMousePos = event->globalPos();
            move(x() + diff.x(), y() + diff.y());
        }
        else if (isResizing)
        {
            QPoint diff = event->globalPos() - lastMousePos;
            lastMousePos = event->globalPos();

            int xNew = x();
            int yNew = y();
            int wid = width();
            int hei = height();

            if (hoverL)
            {
                xNew += diff.x();
                wid -= diff.x();
            }
            else if (hoverR)
                wid += diff.x();

            if (hoverT)
            {
                yNew += diff.y();
                hei -= diff.y();
            }
            else if (hoverB)
                hei += diff.y();

            setGeometry(xNew, yNew, wid, hei);
        }
        else
        {
            hoverL = event->localPos().x() < resizeBorder;
            hoverR = event->localPos().x() > width() - resizeBorder;
            hoverT = event->localPos().y() < resizeBorder;
            hoverB = event->localPos().y() > height() - resizeBorder;

            if (resizeBorder > 0)
            {
                bool hoverL = event->localPos().x() < resizeBorder;
                bool hoverR = event->localPos().x() > width() - resizeBorder;
                bool hoverT = event->localPos().y() < resizeBorder;
                bool hoverB = event->localPos().y() > height() - resizeBorder;

                bool cursorUL = (hoverL && hoverT) || (hoverR && hoverB);
                bool cursorUR = (hoverR && hoverT) || (hoverL && hoverB);

                if (cursorUL)
                    setCursor(QCursor(Qt::SizeFDiagCursor));
                else if (cursorUR)
                    setCursor(QCursor(Qt::SizeBDiagCursor));
                else if (hoverL || hoverR)
                    setCursor(QCursor(Qt::SizeHorCursor));
                else if (hoverT || hoverB)
                    setCursor(QCursor(Qt::SizeVerCursor));
                else
                    unsetCursor();
            }
        }

        ClosableQWidget::mouseMoveEvent(event);
    }

private:
    QPoint lastMousePos;

    bool hoverL = false;
    bool hoverR = false;
    bool hoverT = false;
    bool hoverB = false;
    bool isResizing = false;
    bool isDragging = false;
};
