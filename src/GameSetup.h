#pragma once

#include <vector>
#include <functional>
#include <string>

#include <QWidget>

struct GameSetupMode
{
    std::string Name;
    std::function<void(QWidget *parent)> Creator;
};

class GameSetup
{
public:
    virtual std::string Name() const = 0;
    virtual std::vector<GameSetupMode>& Entries() = 0;
};
