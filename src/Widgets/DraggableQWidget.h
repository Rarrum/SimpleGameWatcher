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

    // base implementation here only stores position and size
    virtual void SaveLayoutIn(std::unordered_map<std::string, std::string> &layoutData) const;
    virtual void RestoreLayoutFrom(const std::unordered_map<std::string, std::string> &layoutData);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint lastMousePos;

    bool hoverL = false;
    bool hoverR = false;
    bool hoverT = false;
    bool hoverB = false;
    bool isResizing = false;
    bool isDragging = false;
};
