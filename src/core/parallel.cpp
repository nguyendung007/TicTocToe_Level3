#include "parallel.h"
#include "logic.h"
#include <algorithm>
#include <vector>

namespace parallel {

static int evalMove(const GameState& state, Move move, char sym, int goal) {
    if (!core::isValidMove(state.board, move)) return -9999999;
    auto nb = core::applyMove(state.board, move, sym);
    if (core::checkWin(nb, sym, goal)) return 100000;
    
    return core::countSymbol(nb, sym);
}
//chạy tuần tự trên 1 CPU
std::vector<ScoredMove> evaluateAllMoves_seq(const GameState& state,
                                               char sym, int goal) {
    auto moves = core::enumerateValidMoves(state.board);
    std::vector<ScoredMove> result;
    result.reserve(moves.size());
    for (auto m : moves) {
        int score = evalMove(state, m, sym, goal);
        result.push_back({m, score});
    }
    std::sort(result.begin(), result.end(),
              [](const ScoredMove& a, const ScoredMove& b) {
                  return a.score > b.score;
              });
    return result;
}

//Chạy song song dù chưa thực sự được cài đặt 
std::vector<ScoredMove> evaluateAllMoves_par(const GameState& state,
                                              char sym, int goal,
                                              int ) {
    
    return evaluateAllMoves_seq(state, sym, goal);
}
}  
