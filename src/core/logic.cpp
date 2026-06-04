#include "logic.h"
#include "pipeline.h"
#include <algorithm>
#include <numeric>

namespace core {

// ── Helpers ──────────────────────────────────────────────────────────────────
static std::vector<int> range(int n) {
    std::vector<int> v(n);
    std::iota(v.begin(), v.end(), 0);
    return v;
}

static Move indexToMove(int idx, int size) {
    return {idx / size, idx % size};
}

// Tạo danh sách tất cả scan-line (startRow, startCol, dr, dc) cho một board
static std::vector<std::tuple<int,int,int,int>> allScanLines(int size) {
    std::vector<std::tuple<int,int,int,int>> lines;
    for (int r = 0; r < size; ++r) lines.emplace_back(r, 0,        0,  1);
    for (int c = 0; c < size; ++c) lines.emplace_back(0, c,        1,  0);
    for (int r = 0; r < size; ++r) lines.emplace_back(r, 0,        1,  1);
    for (int c = 1; c < size; ++c) lines.emplace_back(0, c,        1,  1);
    for (int r = 0; r < size; ++r) lines.emplace_back(r, size - 1, 1, -1);
    for (int c = 0; c < size - 1; ++c) lines.emplace_back(0, c,   1, -1);
    return lines;
}

// ── initBoard ────────────────────────────────────────────────────────────────

Board initBoard(int size) {
    Board b;
    b.size = size;
    auto setEmpty = [size](Board acc, int idx) {
        acc.grid[idx / size][idx % size] = EMPTY_CELL;
        return acc;
    };
    return fp::reduce(b, setEmpty)(range(size * size));
}

// ── isValidMove ──────────────────────────────────────────────────────────────

bool isValidMove(const Board& board, Move move) {
    auto inRange  = [&](Move m) { return board.inRange(m.row, m.col); };
    auto isEmpty  = [&](Move m) { return board.at(m.row, m.col) == EMPTY_CELL; };
    auto validate = fp::pipe(
        [&](Move m) { return std::make_pair(inRange(m), m); },
        [&](std::pair<bool,Move> p) { return p.first && isEmpty(p.second); }
    );
    return validate(move);
}

// ── enumerateCells ───────────────────────────────────────────────────────────

std::vector<Move> enumerateCells(const Board& board) {
    return fp::map([size = board.size](int idx) {
        return indexToMove(idx, size);
    })(range(board.size * board.size));
}

// ── enumerateValidMoves ──────────────────────────────────────────────────────

std::vector<Move> enumerateValidMoves(const Board& board) {
    return fp::pipe(
        [](const Board& b)  { return enumerateCells(b); },
        fp::filter([&board](Move m) { return isValidMove(board, m); })
    )(board);
}

// ── isEmptyHead ──────────────────────────────────────────────────────────────
bool isEmptyHead(const Board& board, int x, int y, char symbol) {
    auto checks = fp::pipe(
        [&](std::pair<int,int> p) -> bool { return !board.inRange(p.first, p.second); },
        [&](bool outOfRange) -> bool {
            if (outOfRange) return true;
            char cell = board.at(x, y);
            return cell == EMPTY_CELL || cell == symbol;
        }
    );
    return checks(std::make_pair(x, y));
}

// ── applyMove ────────────────────────────────────────────────────────────────

Board applyMove(const Board& board, Move move, char symbol) {
    return fp::pipe(
        [](const Board& b) { return b; },
        [&move, symbol](Board next) {
            next.grid[move.row][move.col] = symbol;
            return next;
        }
    )(board);
}

// ── applyTurn ────────────────────────────────────────────────────────────────

GameState applyTurn(const GameState& state, Move move, int goal, EndRule rule) {
    char sym       = symbolOf(state.currentPlayer);
    Board newBoard = applyMove(state.board, move, sym);


    auto withBoard = [&](GameState s) {
        s.board = newBoard;
        s.turn  = state.turn + 1;
        return s;
    };
    auto applyWin = [&](GameState s) {
        if (checkWin(newBoard, sym, goal, rule)) {  
            s.winner = state.currentPlayer;
            s.isFinished = true;
        }
        return s;
    };

    auto applyDraw = [&](GameState s) {
        if (!s.isFinished && checkDraw(newBoard)) {
            s.winner     = DRAW_RESULT;
            s.isFinished = true;
        }
        return s;
    };
    auto applyNext = [&](GameState s) {
        if (!s.isFinished) {
            s.winner        = DRAW_RESULT;
            s.currentPlayer = 1 - state.currentPlayer;
        }
        return s;
    };

    return fp::pipe(withBoard, applyWin, applyDraw, applyNext)(state);
}

// ── checkLine (internal) ─────────────────────────────────────────────────────

static bool checkLine(const Board& board, char symbol, int goal, EndRule rule,
                      int startRow, int startCol, int dr, int dc) {
    int size = board.size;
    for (int r = startRow, c = startCol, k = 0; k <= size - goal;
         r += dr, c += dc, ++k) {
        auto stepIndices = range(goal);
        auto allMatch = fp::reduce(true, [&](bool acc, int step) {
            int nr = r + step * dr, nc = c + step * dc;
            return acc && board.inRange(nr, nc) && board.at(nr, nc) == symbol;
        })(stepIndices);
        if (!allMatch) continue;
        if (rule == EndRule::NONE) return true;
        bool headOpen = isEmptyHead(board, r - dr,        c - dc,        symbol);
        bool tailOpen = isEmptyHead(board, r + goal * dr, c + goal * dc, symbol);
        if (rule == EndRule::OPEN_ONE && (headOpen || tailOpen)) return true;
        if (rule == EndRule::OPEN_TWO && (headOpen && tailOpen)) return true;
    }
    return false;
}

// ── checkWin ─────────────────────────────────────────────────────────────────

bool checkWin(const Board& board, char symbol, int goal, EndRule rule) {
    auto lines = allScanLines(board.size);
    auto results = fp::map([&](auto line) {
        auto [sr, sc, dr, dc] = line;
        return checkLine(board, symbol, goal, rule, sr, sc, dr, dc);
    })(lines);
    return fp::reduce(false, std::logical_or<bool>{})(results);
}

// ── checkDraw ────────────────────────────────────────────────────────────────

bool checkDraw(const Board& board) {
    return fp::pipe(
        [](const Board& b) { return countSymbol(b, EMPTY_CELL); },
        [](int n) { return n == 0; }
    )(board);
}

// ── getWinLine ───────────────────────────────────────────────────────────────

std::optional<WinLine> getWinLine(const Board& board, char symbol, int goal, EndRule rule) {
    int size = board.size;
    auto tryLine = [&](int sr, int sc, int dr, int dc) -> std::optional<WinLine> {
        for (int r = sr, c = sc, k = 0; k <= size - goal; r += dr, c += dc, ++k) {
            auto stepIdx = range(goal);
            bool ok = fp::reduce(true, [&](bool acc, int step) {
                int nr = r + step * dr, nc = c + step * dc;
                return acc && board.inRange(nr, nc) && board.at(nr, nc) == symbol;
            })(stepIdx);
            if (!ok) continue;
            WinLine wl;
            for (int step = 0; step < goal; ++step)
                wl.cells.push_back({r + step * dr, c + step * dc});
            return wl;
        }
        return std::nullopt;
    };

    (void)rule;
    auto lines = allScanLines(size);
    for (auto& [sr, sc, dr, dc] : lines)
        if (auto w = tryLine(sr, sc, dr, dc)) return w;
    return std::nullopt;
}

//Helper TCO 
static int countSymbolTail(const Board& board, char symbol, int index, int count) {
    if (index >= board.size * board.size) {
        return count;
    }
    
    int newCount = count;
    if (board.at(index / board.size, index % board.size) == symbol) {
        newCount = count + 1;
    }
    return countSymbolTail(board, symbol, index + 1, newCount);
}

// ── countSymbol ──────────────────────────────────────────────────────────────

int countSymbol(const Board& board, char symbol) {
    return countSymbolTail(board, symbol, 0, 0);
}

// ── surrender ────────────────────────────────────────────────────────────────

GameState surrender(const GameState& state) {
    return fp::pipe(
        [](GameState s) { s.winner     = 1 - s.currentPlayer; return s; },
        [](GameState s) { s.isFinished = true;                 return s; }
    )(state);
}

} 