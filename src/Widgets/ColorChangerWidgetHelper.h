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

    inline void SaveLayoutIn(std::unordered_map<std::string, std::string> &layoutData) const
    {
        int textR = 0, textG = 0, textB = 0;
        widget->palette().text().color().getRgb(&textR, &textG, &textB);
        layoutData.emplace("textR", std::to_string(textR));
        layoutData.emplace("textG", std::to_string(textG));
        layoutData.emplace("textB", std::to_string(textB));

        int bgR = 0, bgG = 0, bgB = 0;
        widget->palette().window().color().getRgb(&bgR, &bgG, &bgB);
        layoutData.emplace("bgR", std::to_string(bgR));
        layoutData.emplace("bgG", std::to_string(bgG));
        layoutData.emplace("bgB", std::to_string(bgB));

        bool isTransparentBg = widget->testAttribute(Qt::WA_TranslucentBackground);
        layoutData.emplace("bgTransparent", isTransparentBg ? "1" : "0");
    }

    inline void RestoreLayoutFrom(const std::unordered_map<std::string, std::string> &layoutData)
    {
        int textR = std::stoi(layoutData.at("textR"));
        int textG = std::stoi(layoutData.at("textG"));
        int textB = std::stoi(layoutData.at("textB"));
        SetTextColor(QColor(textR, textG, textB));

        int bgR = std::stoi(layoutData.at("bgR"));
        int bgG = std::stoi(layoutData.at("bgG"));
        int bgB = std::stoi(layoutData.at("bgB"));
        SetBackgroundColor(QColor(bgR, bgG, bgB)); //clears transparency

        bool isTransparentBg = std::stoi(layoutData.at("bgTransparent")) != 0;
        if (isTransparentBg)
            SetBackgroundTransparent();
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
