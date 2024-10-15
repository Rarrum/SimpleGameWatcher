#pragma once

#include <string>
#include <unordered_map>

class UpdatableGameWindow
{
public:
    virtual ~UpdatableGameWindow() = default;

    inline virtual void RefreshState() {}
    virtual bool IsStillOpen() const = 0;

    // default implementations save/restore only position and size
    virtual std::unordered_map<std::string, std::string> SaveLayout() const;
    virtual void RestoreLayout(const std::unordered_map<std::string, std::string> &layoutData);
};
