#pragma once
#include <vector>
#include "types.h"

namespace parallel {

struct ScoredMove {
    Move  move;
    int   score;
};
std::vector<ScoredMove> evaluateAllMoves_seq(const GameState& state,
                                              char sym, int goal);

std::vector<ScoredMove> evaluateAllMoves_par(const GameState& state,
                                              char sym, int goal,
                                              int num_threads = 0);
}  
