#include "DebugGameStateWindow.h"

DebugGameStateWindow::DebugGameStateWindow(std::shared_ptr<GameWatcher> gameWatcher, QWidget *parent): ClosableQWidget(parent)
{
    watcher = gameWatcher;

    resize(250, 300);
    setWindowTitle("Debug Game State");
    setWindowFlags(Qt::Window);

    table = new QTableWidget(this);
    table->setColumnCount(2);
    table->setGridStyle(Qt::DotLine);
    table->setWordWrap(false);
    table->setSortingEnabled(false);
    table->setCornerButtonEnabled(false);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem("Watchable Name"));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(table);

    setLayout(layout);

    show();
}

void DebugGameStateWindow::RefreshState()
{
    if (!watcher)
        return;

    std::vector<std::pair<std::string, std::string>> allItems;

    allItems.emplace_back(std::make_pair("(IsReady)", watcher->IsReady() ? "true" : "false"));

    for (const std::string &name : watcher->AllWatchableIntegers())
        allItems.emplace_back(std::make_pair(name, std::to_string(watcher->GetIntegerValue(name))));

    for (const std::string &name : watcher->AllWatchableStrings())
        allItems.emplace_back(std::make_pair(name, watcher->GetStringValue(name)));

    for (const std::string &name : watcher->AllWatchableFlags())
        allItems.emplace_back(std::make_pair(name, watcher->GetFlagValue(name) ? "true" : "false"));

    table->setRowCount((int)allItems.size());
    int row = 0;
    for (const auto &pair : allItems)
    {
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(pair.first)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(pair.second)));
        ++row;
    }
}
