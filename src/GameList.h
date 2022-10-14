#pragma once

#include <vector>
#include <memory>

#include "GameSetup.h"

std::vector<std::unique_ptr<GameSetup>> CreateGameList();