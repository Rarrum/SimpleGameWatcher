
#include "GameList.h"

#include "GameSpecific/Lufia2-GameSetup.h"

std::vector<std::unique_ptr<GameSetup>> CreateGameList()
{
    std::vector<std::unique_ptr<GameSetup>> allGames;
    allGames.emplace_back(std::make_unique<Lufia2GameSetup>());

    return allGames;
}