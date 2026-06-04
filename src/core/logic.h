#pragma once
#include <optional>
#include <vector>
#include "types.h"

namespace core {

Board initBoard(int size);
bool isValidMove(const Board& board, Move move);
std::vector<Move> enumerateCells(const Board& board);
std::vector<Move> enumerateValidMoves(const Board& board);
bool isEmptyHead(const Board& board, int x, int y, char symbol);
Board applyMove(const Board& board, Move move, char symbol);
GameState applyTurn(const GameState& state, Move move, int goal);
bool checkWin(const Board& board, char symbol, int goal,
              EndRule rule = EndRule::OPEN_TWO);
bool checkDraw(const Board& board);
std::optional<WinLine> getWinLine(const Board& board, char symbol, int goal,
                                   EndRule rule = EndRule::OPEN_TWO);
constexpr char symbolOf(int player) noexcept {
    return player == 0 ? SYMBOL_X : SYMBOL_O;
}
constexpr char opponentOf(char symbol) noexcept {
    return symbol == SYMBOL_X ? SYMBOL_O : SYMBOL_X;
}
int countSymbol(const Board& board, char symbol);
GameState surrender(const GameState& state);

GameState applyTurn(const GameState& state, Move move, int goal, EndRule rule = EndRule::OPEN_TWO);
}  

