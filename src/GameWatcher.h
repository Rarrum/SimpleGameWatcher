#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>

class GameWatcher
{
public:
    virtual const std::vector<std::string> AllWatchableIntegers();
    virtual const std::vector<std::string> AllWatchableStrings();
    virtual const std::vector<std::string> AllWatchableFlags();

    virtual int64_t GetIntegerValue(const std::string &name);
    virtual std::string GetStringValue(const std::string &name);
    virtual bool GetFlagValue(const std::string &name);

    virtual bool IsReady() = 0;

protected:
    virtual void PollGameState() = 0;

    void SetIntegerState(const std::string &name, int64_t value);
    void SetStringState(const std::string &name, const std::string &value);
    void SetFlagState(const std::string &name, bool value);

private:
    std::vector<std::string> allIntegerNames;
    std::vector<std::string> allStringNames;
    std::vector<std::string> allFlagNames;

    std::unordered_map<std::string, int64_t> allIntegerValues;
    std::unordered_map<std::string, std::string> allStringValues;
    std::unordered_map<std::string, bool> allFlagValues;

    std::mutex valuesLock;
};
