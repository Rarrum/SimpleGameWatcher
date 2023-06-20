#pragma once

#include <QApplication>
#include <QWidget>
#include <QStyleOption>
#include <QColorDialog>
#include <QMenu>

class ColorChangerWidgetHelper
{
public:
    // Must be called once before use
    inline void SetupColorChanger(QWidget *target)
    {
        widget = target;
        widget->setAutoFillBackground(true);
    }

    inline void PickAndSetBackgroundColor()
    {
        SetBackgroundColor(DoPickColor());
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

    inline void SetSystemDefaultColors()
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
        QColor transparentColor(128, 128, 128, 2); // We really want alpha to be 0, but that breaks mouse clicks for some reason.  Instead we get not-quite-transparent here...
        pal.setColor(QPalette::Window, transparentColor);
        widget->setPalette(pal);
    }

    inline void PickAndSetTextColor()
    {
        SetTextColor(DoPickColor());
    }

    inline void SetTextColor(QColor color)
    {
        if (!color.isValid())
            return;

        QPalette pal = widget->palette();
        pal.setColor(QPalette::Text, color);
        pal.setColor(QPalette::WindowText, color);
        widget->setPalette(pal);
    }

    inline void AddColorOptionsToMenu(QMenu *menu, bool includeBackground, bool includeText)
    {
        if (includeBackground)
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

            menu->addAction(actionSetBgColor);
            menu->addAction(actionSetBgTransparent);
        }

        if (includeText)
        {
            QAction *actionSetTextColor = new QAction("Set Text Color", widget);
            QObject::connect(actionSetTextColor, &QAction::triggered, [&]()
            {
                PickAndSetTextColor();
            });

            menu->addAction(actionSetTextColor);
        }

        if (includeBackground || includeText)
        {
            QAction *actionSetAllDefault = new QAction("Use System Colors", widget);
            QObject::connect(actionSetAllDefault, &QAction::triggered, [&]()
            {
                SetSystemDefaultColors();
            });

            menu->addAction(actionSetAllDefault);
        }
    }

private:
    QWidget *widget = nullptr;

    QColor DoPickColor()
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

        return color;
    }
};
