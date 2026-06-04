#include "bot_pure.h"
#include <algorithm>
#include <climits>
#include <vector>
#include "logic.h"
#include "pipeline.h"
#include "types.h"

namespace bot {

// ── Helpers ──────────────────────────────────────────────────────────────────

static std::vector<std::pair<int,int>> directions() {
    return {{0,1},{1,0},{1,1},{1,-1}};
}

// ── evalPattern ──────────────────────────────────────────────────────────────
// Tách thành 2 hàm thuần: getBase + getMultiplier, kết hợp qua pipe

static int getBase(int length) {
    switch (length) {
        case 5: return 100000;
        case 4: return 10000;
        case 3: return 1000;
        case 2: return 100;
        case 1: return 10;
        default: return 0;
    }
}

static int getMultiplier(int length, bool openLeft, bool openRight, bool isOwn) {
    if (!isOwn) {
        if (length >= 4 && (openLeft || openRight)) return 2;
        if (length >= 3 && openLeft && openRight)   return 3;
        return 1;
    }
    if (openLeft && openRight) return 2;
    if (openLeft || openRight) return 1;
    return -1; 
}

static int evalPattern(int length, bool openLeft, bool openRight, bool isOwn) {
    return fp::pipe(
        [length](bool)      { return getBase(length); },
        [length, openLeft, openRight, isOwn](int base) {
            int mult = getMultiplier(length, openLeft, openRight, isOwn);
            return mult == -1 ? base / 2 : base * mult;
        }
    )(true);
}

//helper
// ── scanDirection ────────────────────────────────────────────────────────────

struct DirScan { int count; 
                 bool openL; 
                 bool openR; };

static DirScan scanDirection(const Board& board, int row, int col,
                             char symbol, int dx, int dy) {
    auto scanSteps = [&](int sign) {
        struct Acc { int cnt; bool open; bool done; };
        return fp::reduce(
            Acc{0, false, false},
            [&](Acc acc, int s) -> Acc {
                if (acc.done) return acc;
                int nr = row + sign * s * dx, nc = col + sign * s * dy;
                if (!board.inRange(nr, nc)) {
                    return {acc.cnt, false, true};
                }
                if (board.at(nr, nc) == symbol) {
                    return {acc.cnt + 1, acc.open, false};
                }
                if (board.at(nr, nc) == EMPTY_CELL) {
                    return {acc.cnt, true, true};
                }
                return {acc.cnt, false, true};
            }
        )(std::vector<int>{1, 2, 3, 4, 5});
    };

    auto fwd = scanSteps(+1);
    auto bwd = scanSteps(-1);
    return {1 + fwd.cnt + bwd.cnt, bwd.open, fwd.open};
}

// ── evalPos ──────────────────────────────────────────────────────────────────

static int evalPos(const Board& board, int row, int col, char symbol) {
    if (board.at(row, col) != EMPTY_CELL) return 0;

    return fp::reduce(0, std::plus<int>{})(
        fp::map([&](std::pair<int,int> d) {
            auto [dx, dy] = d;
            auto scan = scanDirection(board, row, col, symbol, dx, dy);
            return evalPattern(std::min(scan.count, 5), scan.openL, scan.openR, true);
        })(directions())
    );
}

// ── getCandidates ────────────────────────────────────────────────────────────

static std::vector<Move> getCandidates(const Board& board) {
    int size   = board.size;
    int radius = (size <= 8) ? 1 : 2;

    auto occupied = fp::filter([&](Move m) {
        return board.at(m.row, m.col) != EMPTY_CELL;
    })(core::enumerateCells(board));

    std::vector<std::pair<int,int>> seen_guard;
    std::vector<Move> out;

    for (auto& o : occupied) {
        for (int di = -radius; di <= radius; ++di)
        for (int dj = -radius; dj <= radius; ++dj) {
            int ni = o.row + di, nj = o.col + dj;
            if (!board.inRange(ni, nj)) continue;
            if (board.at(ni, nj) != EMPTY_CELL) continue;
            auto key = std::make_pair(ni, nj);
            if (std::find(seen_guard.begin(), seen_guard.end(), key) != seen_guard.end()) continue;
            seen_guard.push_back(key);
            out.push_back({ni, nj});
        }
    }

    if (out.empty()) out.push_back({size / 2, size / 2});
    return out;
}

// ── evalBoard ────────────────────────────────────────────────────────────────

static int evalBoard(const Board& board, char botSym, char oppSym) {
    auto cells = core::enumerateCells(board);

    auto sumFor = [&](char sym) {
        return fp::reduce(0, std::plus<int>{})(
            fp::map([&](Move m) -> int {
                return board.at(m.row, m.col) == sym
                     ? evalPos(board, m.row, m.col, sym) : 0;
            })(cells)
        );
    };

    return sumFor(botSym) - sumFor(oppSym) * 2;
}

// ── minimax ──────────────────────────────────────────────────────────────────

static int minimax(const Board& board, int goal,
                   char botSym, char oppSym,
                   int depth, int alpha, int beta, bool isMax,
                   EndRule rule) {
    if (core::checkWin(board, botSym, goal, rule)) return  100000 + depth;
    if (core::checkWin(board, oppSym, goal, rule)) return -100000 - depth;
    if (core::checkDraw(board))                    return 0;
    if (depth == 0)                                return evalBoard(board, botSym, oppSym);

    char curSym    = isMax ? botSym : oppSym;
    auto candidates = getCandidates(board);

    std::sort(candidates.begin(), candidates.end(), [&](Move a, Move b_) {
        return evalPos(board, a.row, a.col, curSym) >
               evalPos(board, b_.row, b_.col, curSym);
    });

    // fold over candidates with alpha-beta
    if (isMax) {
        return fp::reduce(
            std::make_pair(-9999999, alpha),
            [&](std::pair<int,int> acc, Move m) -> std::pair<int,int> {
                auto [best, a] = acc;
                if (a >= beta) return acc;                   
                Board nb = core::applyMove(board, m, curSym);
                int val  = minimax(nb, goal, botSym, oppSym, depth-1, a, beta, false, rule);
                int nb2  = std::max(best, val);
                return {nb2, std::max(a, nb2)};
            }
        )(candidates).first;
    } else {
        return fp::reduce(
            std::make_pair(9999999, beta),
            [&](std::pair<int,int> acc, Move m) -> std::pair<int,int> {
                auto [best, b] = acc;
                if (alpha >= b) return acc;                    
                Board nb = core::applyMove(board, m, curSym);
                int val  = minimax(nb, goal, botSym, oppSym, depth-1, alpha, b, true, rule);
                int nb2  = std::min(best, val);
                return {nb2, std::min(b, nb2)};
            }
        )(candidates).first;
    }
}

// ── pickRandom ───────────────────────────────────────────────────────────────

static Move pickRandom(const std::vector<Move>& moves, Rng& rng) {
    if (moves.empty()) return INVALID_MOVE;
    return fp::pipe(
        [&](const std::vector<Move>& m) { return m.size(); },
        [&](int size) { 
            std::uniform_int_distribution<int> dist(0, size - 1);
            return dist(rng);
        },
        [&](int idx) { return moves[idx]; }
    )(moves);
}
// ── makeScorer ───────────────────────────────────────────────────────────────

static auto makeScorer(const Board& board, char botSym, char oppSym,
                       int wBot, int wOpp) {
    return [=](Move m) {
        return evalPos(board, m.row, m.col, botSym) * wBot
             + evalPos(board, m.row, m.col, oppSym) * wOpp;
    };
}

// ── easy ─────────────────────────────────────────────────────────────────────

Move easy(const GameState& state, Rng& rng) {
    auto validMoves = fp::pipe(
        [](const Board& b) { return core::enumerateCells(b); },
        fp::filter([&state](Move m) { return core::isValidMove(state.board, m); })
    )(state.board);
    if (validMoves.empty()) return INVALID_MOVE;
    return pickRandom(validMoves, rng);
}

// ── medium ───────────────────────────────────────────────────────────────────

Move medium(const GameState& state, int goal, Rng& rng, EndRule rule) {
    const Board& board = state.board;
    char botSym = core::symbolOf(state.currentPlayer);
    char oppSym = core::opponentOf(botSym);

    auto candidates = getCandidates(board);

    auto winning  = fp::filter([&](Move m) {
        return core::checkWin(core::applyMove(board, m, botSym), botSym, goal, rule);
    })(candidates);
    if (!winning.empty()) return winning[0];

    auto blocking = fp::filter([&](Move m) {
        return core::checkWin(core::applyMove(board, m, oppSym), oppSym, goal, rule);
    })(candidates);
    if (!blocking.empty()) return blocking[0];

    auto scorer = makeScorer(board, botSym, oppSym, 12, 10);

    auto best = fp::reduce(
        std::make_pair(-1, INVALID_MOVE),
        [&scorer](std::pair<int,Move> acc, Move m) {
            int s = scorer(m);
            return s > acc.first ? std::make_pair(s, m) : acc;
        }
    )(candidates);

    if (best.second != INVALID_MOVE) return best.second;
    return easy(state, rng);
}

// ── hard ─────────────────────────────────────────────────────────────────────

Move hard(const GameState& state, int goal, Rng& rng, EndRule rule) {
    (void)rng;
    const Board& board = state.board;
    char botSym = core::symbolOf(state.currentPlayer);
    char oppSym = core::opponentOf(botSym);

    auto candidates = getCandidates(board);

    auto winMove = fp::filter([&](Move m) {
        return core::checkWin(core::applyMove(board, m, botSym), botSym, goal, rule);
    })(candidates);
    if (!winMove.empty()) return winMove[0];

    auto blockMove = fp::filter([&](Move m) {
        return core::checkWin(core::applyMove(board, m, oppSym), oppSym, goal, rule);
    })(candidates);
    if (!blockMove.empty()) return blockMove[0];

    auto scorer = makeScorer(board, botSym, oppSym, 10, 15);

    auto scored = fp::map([&scorer](Move m) {
        return std::make_pair(scorer(m), m);
    })(candidates);

    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    int depth = (board.size <= 5) ? 8 : (board.size <= 8) ? 6 : 4;
    int topN  = std::min((int)scored.size(), 12);

    auto result = fp::reduce(
        std::make_pair(-9999999, scored[0].second),
        [&](std::pair<int,Move> acc, std::pair<int,Move> sm) {
            Move m   = sm.second;
            Board nb = core::applyMove(board, m, botSym);
            int val  = minimax(nb, goal, botSym, oppSym,
                               depth, -9999999, 9999999, false, rule);
            return val > acc.first ? std::make_pair(val, m) : acc;
        }
    )(std::vector<std::pair<int,Move>>(scored.begin(), scored.begin() + topN));

    return result.second;
}

// ── makeBot ──────────────────────────────────────────────────────────────────

BotFn makeBot(BotLevel level, int goal, EndRule rule) {
    switch (level) {
        case BotLevel::EASY:
            return [](const GameState& s, Rng& r) { return easy(s, r); };
        case BotLevel::MEDIUM:
            return [goal, rule](const GameState& s, Rng& r) { return medium(s, goal, r, rule); };
        case BotLevel::HARD:
            return [goal, rule](const GameState& s, Rng& r) { 
                return hard(s, goal, r, rule);  
            };
        default:
            return [](const GameState&, Rng&) { return INVALID_MOVE; };
    }
}
} 

