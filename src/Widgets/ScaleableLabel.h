#pragma once

#include <QLabel>
#include <QFont>

class ScaleableLabel: public QLabel
{
public:
    inline ScaleableLabel(const std::string &text, QWidget *parent = nullptr): QLabel(QString::fromStdString(text), parent)
    {
        setMinimumHeight(5);

        for (char c : text)
        {
            if (c == '\n')
                ++linesOfText;
        }
    }

    float FillFactor = 1.0f;

    inline void ChangeAndScaleFont(QFont newFont)
    {
        if (text().isEmpty())
            return;

        QRect labelRect = contentsRect();
        labelRect.setHeight((int)(FillFactor * labelRect.height() / linesOfText));

        int fontSize = newFont.pixelSize();
        if (fontSize < 5)
        {
            fontSize = 5;
            newFont.setPixelSize(fontSize);
        }

        QRect textRect = QFontMetrics(newFont).boundingRect(text());

        if (textRect.height() < labelRect.height() && textRect.width() < labelRect.width())
        {
            while (fontSize < 100)
            {
                ++fontSize;
                newFont.setPixelSize(fontSize);
                textRect = QFontMetrics(newFont).boundingRect(text());
                if (textRect.height() > labelRect.height() || textRect.width() > labelRect.width())
                {
                    --fontSize;
                    newFont.setPixelSize(fontSize);
                    break;
                }
            }
        }
        else if (textRect.height() > labelRect.height() || textRect.width() > labelRect.width())
        {
            while (fontSize > 5)
            {
                --fontSize;
                newFont.setPixelSize(fontSize);
                textRect = QFontMetrics(newFont).boundingRect(text());
                if (textRect.height() < labelRect.height() && textRect.width() < labelRect.width())
                    break;
            }
        }

        setFont(newFont);
    }

protected:
    inline void resizeEvent(QResizeEvent *event) override
    {
        ChangeAndScaleFont(font());

        QLabel::resizeEvent(event);
    }

private:
    int linesOfText = 1;
};
