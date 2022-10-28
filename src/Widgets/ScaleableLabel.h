#pragma once

#include <QLabel>
#include <QFont>

class ScaleableLabel: public QLabel
{
public:
    inline public ScaleableLabel(const std::string &text, QWidget *parent = nullptr): QLabel(QString::fromStdString(text), parent)
    {
        setMinimumHeight(5);

        for (char c : text)
        {
            if (c == '\n')
                ++linesOfText;
        }
    }

    float FillFactor = 1.0f;

    inline int ScaleToFit()
    {
        if (text().isEmpty())
            return font().pixelSize();

        QFont newFont = font();
        QRect labelRect = contentsRect();
        labelRect.setHeight((int)(FillFactor * labelRect.height() / linesOfText));

        int fontSize = newFont.pixelSize();
        if (fontSize < 5)
        {
            fontSize = 5;
            newFont.setPixelSize(fontSize);
        }

        QRect textRect = QFontMetrics(newFont).boundingRect(text());

        if (textRect.height() < labelRect.height())
        {
            while (fontSize < 100)
            {
                ++fontSize;
                newFont.setPixelSize(fontSize);
                textRect = QFontMetrics(newFont).boundingRect(text());
                if (textRect.height() > labelRect.height())
                {
                    --fontSize;
                    newFont.setPixelSize(fontSize);
                    break;
                }
            }
        }
        else if (textRect.height() > labelRect.height())
        {
            while (fontSize > 5)
            {
                --fontSize;
                newFont.setPixelSize(fontSize);
                textRect = QFontMetrics(newFont).boundingRect(text());
                if (textRect.height() < labelRect.height())
                    break;
            }
        }

        setFont(newFont);
        return fontSize;
    }

protected:
    inline void resizeEvent(QResizeEvent *event) override
    {
        ScaleToFit();

        QLabel:: resizeEvent(event);
    }

private:
    int linesOfText = 1;
};
