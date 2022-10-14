#include <vector>
#include <list>
#include <memory>

#include <QApplication>
#include <QPushButton>
#include <QObject>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QStringListModel>
#include <QCompleter>

#include "ClosableQWidget.h"
#include "SimpleTimer.h"

#include "GameList.h"

int main(int argc, char **argv)
{
    std::vector<std::unique_ptr<GameSetup>> allGames = CreateGameList();

    //main window
    QApplication app(argc, argv);
    ClosableQWidget window = ClosableQWidget();
    ClosableQWidget *windowPointer = &window;
    window.resize(500, 400);

    QGroupBox *manualBox = new QGroupBox("Manual Controlled");
    QGroupBox *autoBox = new QGroupBox("Auto Controlled");
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(manualBox);
    topLayout->addWidget(autoBox);

    QVBoxLayout *manualLayout = new QVBoxLayout();
    manualLayout->setAlignment(Qt::AlignTop);
    QVBoxLayout *autoLayout = new QVBoxLayout();;
    autoLayout->setAlignment(Qt::AlignTop);

    // manual section
    QPushButton *createManualTimerButton = new QPushButton(&window);
    createManualTimerButton->setText("Create Simple Timer");
    QObject::connect(createManualTimerButton, &QPushButton::clicked, [&]()
    {
        SimpleTimer *newTimer = new SimpleTimer(true, &window);
    });
    manualLayout->addWidget(createManualTimerButton);

    // auto section
    QComboBox *comboGameSelect = new QComboBox(&window);
    comboGameSelect->setEditable(true);
    QStringList gameList;
    for (auto &gameSetup : allGames)
    {
        comboGameSelect->addItem(QString::fromStdString(gameSetup->Name()));
        gameList.append(QString::fromStdString(gameSetup->Name()));
    }
    QStringListModel *gameListModel = new QStringListModel(gameList, &window);
    QCompleter *gameSelectCompleter = new QCompleter(&window);
    gameSelectCompleter->setCompletionMode(QCompleter::InlineCompletion);
    gameSelectCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    gameSelectCompleter->setModel(gameListModel);
    comboGameSelect->setInsertPolicy(QComboBox::NoInsert);
    comboGameSelect->setCompleter(gameSelectCompleter);
    autoLayout->addWidget(comboGameSelect);

    std::vector<QPushButton*> gameActionButtons;
    auto currentGameIndedChangedAction = [&](int index)
    {
        for (QPushButton *b : gameActionButtons)
        {
            autoLayout->removeWidget(b);
            delete b;
        }
        gameActionButtons.clear();

        if (index < 0 || index >= allGames.size())
            return;

        auto &gameSetup = allGames[index];
        for (GameSetupMode &gameMode : gameSetup->Entries())
        {
            QPushButton *gameActionButton = new QPushButton();
            gameActionButton->setText(QString::fromStdString("Create " + gameMode.Name));
            GameSetupMode *gameModePointer = &gameMode;
            QObject::connect(gameActionButton, &QPushButton::clicked, [=]()
            {
                gameModePointer->Creator(windowPointer);
            });

            autoLayout->addWidget(gameActionButton);
            gameActionButtons.emplace_back(gameActionButton);
        }
    };

    QObject::connect(comboGameSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), currentGameIndedChangedAction);
    currentGameIndedChangedAction(0);

    //
    manualBox->setLayout(manualLayout);
    autoBox->setLayout(autoLayout);

    window.setWindowTitle("EasyAutoTracker");
    window.setLayout(topLayout);
    window.show();

    return app.exec();
}
