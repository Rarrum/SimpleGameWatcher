#pragma once

#include <string>

class UpdatableGameWindow
{
public:
    virtual ~UpdatableGameWindow() = default;

    inline virtual void RefreshState() {}
    virtual bool IsStillOpen() const = 0;

    // default implementations save/restore only position and size
    virtual std::string SaveLayout();
    virtual void RestoreLayout(const std::string &layoutData);
};
