#include "GameWatcher.h"

const std::vector<std::string> GameWatcher::AllWatchableIntegers()
{
    std::lock_guard mut(valuesLock);

    return allIntegerNames;
}

const std::vector<std::string> GameWatcher::AllWatchableStrings()
{
    std::lock_guard mut(valuesLock);

    return allStringNames;
}

const std::vector<std::string> GameWatcher::AllWatchableFlags()
{
    std::lock_guard mut(valuesLock);

    return allFlagNames;
}

int64_t GameWatcher::GetIntegerValue(const std::string &name)
{
    std::lock_guard mut(valuesLock);

    auto found = allIntegerValues.find(name);
    if (found != allIntegerValues.end())
        return found->second;
    else
        return 0;
}

std::string GameWatcher::GetStringValue(const std::string &name)
{
    std::lock_guard mut(valuesLock);

    auto found = allStringValues.find(name);
    if (found != allStringValues.end())
        return found->second;
    else
        return std::string();
}

bool GameWatcher::GetFlagValue(const std::string &name)
{
    std::lock_guard mut(valuesLock);

    auto found = allFlagValues.find(name);
    if (found != allFlagValues.end())
        return found->second;
    else
        return false;
}

void GameWatcher::SetIntegerState(const std::string &name, int64_t value)
{
    std::lock_guard mut(valuesLock);

    auto found = allIntegerValues.find(name);
    if (found != allIntegerValues.end())
        found->second = value;
    else
    {
        allIntegerNames.emplace_back(name);
        allIntegerValues.emplace(name, value);
    }
}

void GameWatcher::SetStringState(const std::string &name, const std::string &value)
{
    std::lock_guard mut(valuesLock);

    auto found = allStringValues.find(name);
    if (found != allStringValues.end())
        found->second = value;
    else
    {
        allStringNames.emplace_back(name);
        allStringValues.emplace(name, value);
    }
}

void GameWatcher::SetFlagState(const std::string &name, bool value)
{
    std::lock_guard mut(valuesLock);

    auto found = allFlagValues.find(name);
    if (found != allFlagValues.end())
        found->second = value;
    else
    {
        allFlagNames.emplace_back(name);
        allFlagValues.emplace(name, value);
    }
}
