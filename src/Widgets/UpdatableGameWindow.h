#pragma once

class UpdatableGameWindow
{
public:
    virtual ~UpdatableGameWindow() = default;

    inline virtual void RefreshState() {}
    inline virtual bool IsStillOpen() const = 0;
};
