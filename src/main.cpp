#include <vector>
#include <list>
#include <memory>

#include <QApplication>
#include <QPushButton>
#include <QObject>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QStringListModel>
#include <QCompleter>
#include <QLabel>

#include "ClosableQWidget.h"
#include "SimpleTimerWindow.h"

#include "GameList.h"

int main(int argc, char **argv)
{
    std::vector<std::unique_ptr<GameSetup>> allGames = CreateGameList();
    std::vector<std::unique_ptr<SimpleTimerWindow>> manualTimers;

    GameSetup *activeGame = nullptr;
    std::function<void()> onEnableGameAuto;
    std::function<void()> onDisableGameAuto;

    // main window
    QApplication app(argc, argv);
    ClosableQWidget window = ClosableQWidget();
    window.resize(450, 400);

    // manual section
    QVBoxLayout *manualLayout = new QVBoxLayout();
    manualLayout->setAlignment(Qt::AlignTop);
    QGroupBox *manualBox = new QGroupBox("Create Manual Controlled");
    manualBox->setLayout(manualLayout);
    
    QPushButton *createManualTimerButton = new QPushButton(&window);
    createManualTimerButton->setText("Simple Timer");
    QObject::connect(createManualTimerButton, &QPushButton::clicked, [&]()
    {
        manualTimers.emplace_back(std::make_unique<SimpleTimerWindow>(true));
    });
    manualLayout->addWidget(createManualTimerButton);

    // auto section
    QGroupBox *autoBox = new QGroupBox("Create Auto Controlled");
    QVBoxLayout *autoLayout = new QVBoxLayout();;
    autoLayout->setAlignment(Qt::AlignTop);
    autoBox->setLayout(autoLayout);
    
    std::vector<QPushButton*> gameActionButtons;
    auto currentGameIndedChangedAction = [&](int index)
    {
        for (QPushButton *b : gameActionButtons)
        {
            autoLayout->removeWidget(b);
            delete b;
        }
        gameActionButtons.clear();

        if (index < 0 || index >= (int)allGames.size())
            return;

        activeGame = allGames[index].get();

        auto &gameSetup = allGames[index];
        for (GameSetupMode &gameMode : gameSetup->Entries())
        {
            QPushButton *gameActionButton = new QPushButton();
            gameActionButton->setText(QString::fromStdString(gameMode.Name));
            gameActionButton->setEnabled(false);
            GameSetupMode *gameModePointer = &gameMode;
            QObject::connect(gameActionButton, &QPushButton::clicked, [=]()
            {
                gameModePointer->Creator();
            });

            autoLayout->addWidget(gameActionButton);
            gameActionButtons.emplace_back(gameActionButton);
        }

        //every game gets a debug mode too
        QPushButton *gameDebugButton = new QPushButton();
        gameDebugButton->setText("Debug");
        gameDebugButton->setEnabled(false);
        GameSetup *gameSetupPointer = gameSetup.get();
        QObject::connect(gameDebugButton, &QPushButton::clicked, [=]()
        {
            gameSetupPointer->CreateDebugWindow();
        });
        autoLayout->addWidget(gameDebugButton);
        gameActionButtons.emplace_back(gameDebugButton);
    };

    // game select section
    QComboBox *comboGameSelect = new QComboBox(&window);
    comboGameSelect->setEditable(true);
    QStringList gameList;
    for (auto &gameSetup : allGames)
    {
        std::string gameName = gameSetup->Name();
        comboGameSelect->addItem(QString::fromStdString(gameName));
        gameList.append(QString::fromStdString(gameName));
    }
    QStringListModel *gameListModel = new QStringListModel(gameList, &window);
    QCompleter *gameSelectCompleter = new QCompleter(&window);
    gameSelectCompleter->setCompletionMode(QCompleter::InlineCompletion);
    gameSelectCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    gameSelectCompleter->setModel(gameListModel);
    comboGameSelect->setInsertPolicy(QComboBox::NoInsert);
    comboGameSelect->setCompleter(gameSelectCompleter);

    QObject::connect(comboGameSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), currentGameIndedChangedAction);
    currentGameIndedChangedAction(0);

    QLabel *labelGameSelect = new QLabel("Pick Game:");
    labelGameSelect->setBuddy(comboGameSelect);

    QGridLayout *gameSelectLayout = new QGridLayout();
    gameSelectLayout->addWidget(labelGameSelect, 0, 0);
    gameSelectLayout->addWidget(comboGameSelect, 0, 1, 1, 2);

    QPushButton *activateGameButton = new QPushButton(&window);
    activateGameButton->setText("Enable Auto For Game");
    QObject::connect(activateGameButton, &QPushButton::clicked, [&]()
    {
        if (comboGameSelect->isEnabled())
            onEnableGameAuto();
        else
            onDisableGameAuto();
    });
    gameSelectLayout->addWidget(activateGameButton, 1, 1);

    QLabel *activeGameStatusLabel = new QLabel();
    gameSelectLayout->addWidget(activeGameStatusLabel, 1, 2);

    onEnableGameAuto = [&]()
    {
        activateGameButton->setText("Stop Auto And Reset");
        comboGameSelect->setEnabled(false);

        activeGame->OnWatcherUpdate = [&]()
        {
            bool isReady = activeGame->Watcher() && activeGame->Watcher()->IsReady();
            std::string warning = activeGame->Watcher() ? activeGame->Watcher()->GetLastWarning() : std::string();

            if (isReady)
            {
                if (warning.empty())
                {
                    activeGameStatusLabel->setText("Ready");
                    activeGameStatusLabel->setStyleSheet("QLabel { color : green; }");
                }
                else
                {
                    activeGameStatusLabel->setText(QString::fromStdString(warning));
                    activeGameStatusLabel->setStyleSheet("QLabel { color : goldenrod; }");
                }
            }
            else
            {
                if (warning.empty())
                {
                    activeGameStatusLabel->setText("Scanning...");
                    activeGameStatusLabel->setStyleSheet("QLabel { color : red; }");
                }
                else
                {
                    activeGameStatusLabel->setText(QString::fromStdString(warning));
                    activeGameStatusLabel->setStyleSheet("QLabel { color : red; }");
                }
            }
        };

        activeGame->StartWatching();

        for (QPushButton *gameAutoButton : gameActionButtons)
            gameAutoButton->setEnabled(true);
    };

    onDisableGameAuto = [&]()
    {
        activateGameButton->setText("Enable Auto For Game");
        comboGameSelect->setEnabled(true);
        activeGame->CloseWindowsAndStopWatching();

        activeGame->OnWatcherUpdate = nullptr;

        for (QPushButton *gameAutoButton : gameActionButtons)
            gameAutoButton->setEnabled(false);

        activeGameStatusLabel->setText("");
    };

    QGroupBox *gameSetupBox = new QGroupBox("Game Selection");
    gameSetupBox->setLayout(gameSelectLayout);

    // main window layout
    QHBoxLayout *createButtonsLayout = new QHBoxLayout();
    createButtonsLayout->addWidget(manualBox);
    createButtonsLayout->addWidget(autoBox);

    QVBoxLayout *topLayout = new QVBoxLayout();
    topLayout->addWidget(gameSetupBox, 1);
    topLayout->addLayout(createButtonsLayout, 999);

    window.setLayout(topLayout);
    window.setWindowTitle("EasyAutoTracker");
    window.show();

    // run and shutdown
    window.closeCallback = [&](ClosableQWidget&)
    {
        allGames.clear();
        manualTimers.clear();
    };

    return app.exec();
}
