#pragma once
#include <functional>
#include "../shell/rng.h"
#include "types.h"

namespace bot {

using BotFn = std::function<Move(const GameState&, Rng&)>;
using SymbolicBot = std::function<Move(const GameState&, char symbol, int goal, Rng&)>;

Move easy(const GameState& state, Rng& rng);
Move medium(const GameState& state, int goal, Rng& rng, EndRule rule);
Move hard(const GameState& state, int goal, Rng& rng, EndRule rule);

BotFn makeBot(BotLevel level, int goal, EndRule rule);

} 
