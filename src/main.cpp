#include <vector>
#include <list>
#include <memory>
#include <fstream>

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
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>

#include "Widgets/ClosableQWidget.h"
#include "Widgets/SimpleTimerWindow.h"
#include "Widgets/CounterWindow.h"

#include "GameList.h"

namespace
{
    std::string ReadFileFromDisk(const std::string &fileName)
    {
        std::string ret;

        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::streampos startPos = file.tellg();
            file.seekg(0, std::ios::end);
            std::streampos endPos = file.tellg();
            file.seekg(startPos, std::ios::beg);

            ret.reserve(endPos - startPos);
            ret.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        }

        return ret;
    }

    bool SaveFileToDisk(const std::string &fileName, const std::string &data)
    {
        std::ofstream file(fileName.c_str(), std::ios::out | std::ios::binary);
        if (!file.is_open())
            return false;

        file.write(data.data(), data.size());
        return true;
    }
}

int main(int argc, char **argv)
{
    std::vector<std::unique_ptr<GameSetup>> allGames = CreateGameList();
    std::vector<std::unique_ptr<SimpleTimerWindow>> manualTimers;
    std::vector<std::unique_ptr<CounterWindow>> manualCounters;

    GameSetup *activeGame = nullptr;
    std::function<void()> onEnableGameAuto;
    std::function<void()> onDisableGameAuto;

    // main window
    QApplication app(argc, argv);
    ClosableQWidget window = ClosableQWidget();
    window.resize(500, 400);

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

    QPushButton *createManualCounterButton = new QPushButton(&window);
    createManualCounterButton->setText("Simple Counter");
    QObject::connect(createManualCounterButton, &QPushButton::clicked, [&]()
    {
        manualCounters.emplace_back(std::make_unique<CounterWindow>(true));
    });
    manualLayout->addWidget(createManualCounterButton);

    // auto section
    QGroupBox *autoBox = new QGroupBox("Create Game Auto Controlled");
    QVBoxLayout *autoLayout = new QVBoxLayout();
    autoLayout->setAlignment(Qt::AlignTop);
    autoBox->setLayout(autoLayout);

    //options section
    QGroupBox *optionsBox = new QGroupBox("Game Auto Options");
    QVBoxLayout *optionsLayout = new QVBoxLayout();
    optionsLayout->setAlignment(Qt::AlignTop);
    optionsBox->setLayout(optionsLayout);

    // game change handler
    std::vector<QPushButton*> gameActionButtons;
    std::vector<QCheckBox*> gameOptionCheckers;
    auto currentGameIndedChangedAction = [&](int index)
    {
        // clear old game actions and options
        for (QPushButton *b : gameActionButtons)
        {
            autoLayout->removeWidget(b);
            delete b;
        }
        gameActionButtons.clear();
        
        for (QCheckBox *b : gameOptionCheckers)
        {
            optionsLayout->removeWidget(b);
            delete b;
        }
        gameOptionCheckers.clear();

        // change game
        if (index < 0 || index >= (int)allGames.size())
            return;

        activeGame = allGames[index].get();

        // add new game actions buttons
        auto &gameSetup = allGames[index];
        for (const GameSetupMode &gameMode : gameSetup->GameModes())
        {
            QPushButton *gameActionButton = new QPushButton();
            gameActionButton->setText(QString::fromStdString(gameMode.Name));
            gameActionButton->setEnabled(false);

            const GameSetupMode *gameModePointer = &gameMode;
            QObject::connect(gameActionButton, &QPushButton::clicked, [=]()
            {
                gameModePointer->Creator();
            });

            autoLayout->addWidget(gameActionButton);
            gameActionButtons.emplace_back(gameActionButton);
        }

        // (every game gets a debug mode too)
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

        // add new game options boxes
        for (GameSetupOption &gameOption : gameSetup->GameOptions())
        {
            QCheckBox *gameOptionBox = new QCheckBox();
            gameOptionBox->setText(QString::fromStdString(gameOption.Name));
            gameOptionBox->setEnabled(false);
            gameOptionBox->setCheckState(gameOption.Enabled ? Qt::Checked : Qt::Unchecked);

            GameSetupOption *gameOptionPointer = &gameOption;
            QObject::connect(gameOptionBox, &QCheckBox::stateChanged, [=]()
            {
                gameOptionPointer->Enabled = gameOptionBox->isChecked();
                gameOptionPointer->OptionChanged(gameOptionPointer->Enabled);
            });

            optionsLayout->addWidget(gameOptionBox);
            gameOptionCheckers.emplace_back(gameOptionBox);
        }
    };

    // load and save buttons (added in the windows layouts section below)
    QPushButton *saveLayoutButton = new QPushButton(&window);
    saveLayoutButton->setText("Save");
    saveLayoutButton->setEnabled(false);
    QObject::connect(saveLayoutButton, &QPushButton::clicked, [&]()
    {
        if (activeGame)
        {
            std::string fileName = QFileDialog::getSaveFileName(&window, "Save Layout", QString(), "Layout (*.layout.json)").toStdString();
            if (!fileName.empty())
            {
                std::string data = activeGame->SaveLayout();
                if (!SaveFileToDisk(fileName, data))
                {
                    QMessageBox::warning(&window, "Not Good", "Failed to write file to disk.");
                }
            }
        }
    });

    QPushButton *loadLayoutButton = new QPushButton(&window);
    loadLayoutButton->setText("Load");
    loadLayoutButton->setEnabled(false);
    QObject::connect(loadLayoutButton, &QPushButton::clicked, [&]()
    {
        if (activeGame)
        {
            std::string fileName = QFileDialog::getOpenFileName(&window, "Load Layout", QString(), "Layout (*.layout.json)").toStdString();
            if (!fileName.empty())
            {
                std::string data = ReadFileFromDisk(fileName);
                if (data.empty())
                    QMessageBox::warning(&window, "Not Good", "Failed to read file from disk.");
                else
                {
                    try
                    {
                        activeGame->RestoreLayout(data);
                    }
                    catch (const std::exception &ex)
                    {
                        QMessageBox::warning(&window, "Not Good", QString("Error restoring layout data: ") + ex.what());
                    }

                    for (QCheckBox *optionsBox : gameOptionCheckers)
                    {
                        auto optionIter = std::find_if(activeGame->GameOptions().begin(), activeGame->GameOptions().end(), [&](const auto &option) { return optionsBox->text().toStdString() == option.Name; });
                        if (optionIter !=activeGame->GameOptions().end())
                            optionsBox->setCheckState(optionIter->Enabled ? Qt::Checked : Qt::Unchecked);
                    }
                }
            }
        }
    });

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

        loadLayoutButton->setEnabled(true);
        saveLayoutButton->setEnabled(true);

        activeGameStatusLabel->setText("Starting scan...");
        activeGameStatusLabel->setStyleSheet("QLabel { color : blue; }");

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

        for (QCheckBox *gameOptionBox : gameOptionCheckers)
            gameOptionBox->setEnabled(true);
    };

    onDisableGameAuto = [&]()
    {
        activateGameButton->setText("Enable Auto For Game");
        comboGameSelect->setEnabled(true);

        loadLayoutButton->setEnabled(false);
        saveLayoutButton->setEnabled(false);

        activeGame->CloseWindowsAndStopWatching();
        activeGame->OnWatcherUpdate = nullptr;

        for (QPushButton *gameAutoButton : gameActionButtons)
            gameAutoButton->setEnabled(false);

        for (QCheckBox *gameOptionBox : gameOptionCheckers)
            gameOptionBox->setEnabled(false);

        activeGameStatusLabel->setText("");
    };

    QGroupBox *gameSetupBox = new QGroupBox("Game Selection");
    gameSetupBox->setLayout(gameSelectLayout);

    // manual and auto options/creation layout
    QWidget *leftCreationOptionsHolder = new QWidget();
    QVBoxLayout *leftCreationOptionsLayout = new QVBoxLayout();
    leftCreationOptionsLayout->setContentsMargins(0, 0, 0, 0); // fix the left section being "smaller" than the right
    leftCreationOptionsLayout->addWidget(manualBox);
    leftCreationOptionsLayout->addWidget(optionsBox);
    leftCreationOptionsHolder->setLayout(leftCreationOptionsLayout);

    // layouts layout
    QGroupBox *loadSaveLayoutsBox = new QGroupBox("Window Layouts (Auto Only)");
    QHBoxLayout *loadSaveLayoutsLayout = new QHBoxLayout();

    loadSaveLayoutsLayout->addWidget(saveLayoutButton);
    loadSaveLayoutsLayout->addWidget(loadLayoutButton);
    loadSaveLayoutsBox->setLayout(loadSaveLayoutsLayout);

    // main window layout
    QHBoxLayout *createButtonsLayout = new QHBoxLayout();
    createButtonsLayout->addWidget(leftCreationOptionsHolder);
    createButtonsLayout->addWidget(autoBox);

    QVBoxLayout *topLayout = new QVBoxLayout();
    topLayout->addWidget(gameSetupBox, 1);
    topLayout->addLayout(createButtonsLayout, 999);
    topLayout->addWidget(loadSaveLayoutsBox, 1);

    window.setLayout(topLayout);
    window.setWindowTitle("EasyAutoTracker");
    window.show();

    // run and shutdown
    window.closeCallback = [&](ClosableQWidget&)
    {
        allGames.clear();
        manualTimers.clear();
        manualCounters.clear();
    };

    return app.exec();
}
