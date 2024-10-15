#include "UpdatableGameWindow.h"

std::unordered_map<std::string, std::string> UpdatableGameWindow::SaveLayout() const
{
    //TODO
    return
    {
        { "x", "12345"}
    };
}

void UpdatableGameWindow::RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData)
{
    //TODO
}
