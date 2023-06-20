#pragma once

#include <QApplication>
#include <QWidget>
#include <QStyleOption>
#include <QColorDialog>
#include <QMenu>

class BackgroundChangerWidgetHelper
{
public:
    // Must be called once before use
    inline void SetupBackgroundChanger(QWidget *target)
    {
        widget = target;
        widget->setAutoFillBackground(true);
    }

    inline void PickAndSetBackgroundColor()
    {
        bool hasAlwaysOnTopHint = (widget->windowFlags() & Qt::WindowStaysOnTopHint) != 0;
        if (hasAlwaysOnTopHint)
        {
            widget->setWindowFlags(widget->windowFlags() & ~Qt::WindowStaysOnTopHint);
            widget->show();
        }

        QColor color = QColorDialog::getColor();

        if (hasAlwaysOnTopHint)
        {
            widget->setWindowFlags(widget->windowFlags() | Qt::WindowStaysOnTopHint);
            widget->show();
        }

        SetBackgroundColor(color);
    }

    inline void SetBackgroundColor(QColor color)
    {
        if (!color.isValid())
            return;

        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        widget->setAutoFillBackground(true);
        QPalette pal = widget->palette();
        pal.setColor(QPalette::Window, color);
        widget->setPalette(pal);
    }

    inline void SetSystemDefaultBackgroundColor()
    {
        widget->setAttribute(Qt::WA_TranslucentBackground, false);

        widget->setAutoFillBackground(true);
        widget->setPalette(QApplication::style()->standardPalette());
    }

    // Note, Qt::FramelessWindowHint must be set for this to work right on windows
    inline void SetBackgroundTransparent()
    {
        widget->setAttribute(Qt::WA_TranslucentBackground, true);
        widget->setAttribute(Qt::WA_NoSystemBackground, false);

        QPalette pal = widget->palette();
        QColor transparentColor(200, 200, 200, 2); // We really want alpha to be 0, but that breaks mouse clicks for some reason.  Instead we get not-quite-transparent here...
        pal.setColor(QPalette::Window, transparentColor);
        widget->setPalette(pal);
    }

    inline void AddBackgroundOptionsToMenu(QMenu *menu)
    {
        QAction *actionSetBgColor = new QAction("Set Background Color", widget);
        QObject::connect(actionSetBgColor, &QAction::triggered, [&]()
        {
            PickAndSetBackgroundColor();
        });

        QAction *actionSetBgTransparent = new QAction("Set Background Transparent", widget);
        QObject::connect(actionSetBgTransparent, &QAction::triggered, [&]()
        {
            SetBackgroundTransparent();
        });

        QAction *actionSetBgDefault = new QAction("Set Background Default", widget);
        QObject::connect(actionSetBgDefault, &QAction::triggered, [&]()
        {
            SetSystemDefaultBackgroundColor();
        });

        menu->addAction(actionSetBgColor);
        menu->addAction(actionSetBgTransparent);
        menu->addAction(actionSetBgDefault);
    }

private:
    QWidget *widget = nullptr;
};
