#include "renderer.h"
#include <iomanip>
#include <iostream>
#include <string>

namespace {
const std::string RESET  = "\033[0m";
const std::string RED    = "\033[31m";
const std::string GREEN  = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN   = "\033[36m";
}  

static bool s_judge_mode = false;

TerminalRenderer::TerminalRenderer() : I_Renderer() {}
TerminalRenderer::~TerminalRenderer() = default;

void TerminalRenderer::init(const RunConfig& config) {
    s_judge_mode = config.judge_mode;
}

void TerminalRenderer::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\x1B[2J\x1B[H" << std::flush;
#endif
}

void TerminalRenderer::showSelectMenu(SelectType selectType, int context) {
    if (s_judge_mode) return;   
    switch (selectType) {
        case SelectType::TITLE_UI:
            std::cout << ">----- Tic-tac-toe [Console v" << VERSION << "] -----<\n\n";
            break;

        case SelectType::SIZE_UI:
            std::cout << "Enter board size NxN (3 <= N <= " << BOARD_N_MAX << "): ";
            break;

        case SelectType::GOAL_UI:
            std::cout << "Enter consecutive pieces to win (3 <= goal <= "
                      << context << "): ";
            break;

        case SelectType::GAME_MODE_UI:
            std::cout << "Select game mode:\n"
                      << "  (1) PvP  - Player vs Player\n"
                      << "  (2) PvE  - Player vs Bot\n"
                      << "  (3) EvE  - Bot vs Bot\n"
                      << "Choice: ";
            break;

     case SelectType::ENDRULE_UI:  
            std::cout << "\n=== SELECT ENDRULE ===\n";
            std::cout << "1. NONE \n";
            std::cout << "2. OPEN_ONE \n";
            std::cout << "3. OPEN_TWO \n";
            std::cout << "SELECT (1-3): ";
            break;

        case SelectType::BOT_LEVEL_UI:
            std::cout << "Select bot difficulty:\n"
                      << "  (1) EASY\n"
                      << "  (2) MEDIUM\n"
                      << "  (3) HARD\n"
                      << "Choice: ";
            break;

        case SelectType::PLAYER_UI:
            std::cout << "Enter move (row col)  |  u=undo  |  s=surrender : ";
            break;

        case SelectType::MUL_BOT_LEVEL_UI:
            std::cout << "Select difficulty for Bot:\n"
                      << "  (1) EASY  (2) MEDIUM  (3) HARD\n"
                      << "Choice: ";
            break;

        default:
            break;
    }
}

void TerminalRenderer::showInvalidSelect(SelectType , int ) {
    if (s_judge_mode) return;
    std::cout << "Invalid choice. Please try again.\n";
}

void TerminalRenderer::showValidSelect(SelectType , int ) {
    
}

void TerminalRenderer::displayBoard(const Board& board) {
    if (s_judge_mode) return;
    int size = board.size;
    int indent = 4;
    std::string pad(indent, ' ');

    
    std::cout << pad << "    ";
    for (int c = 0; c < size; ++c)
        std::cout << std::setw(3) << c;
    std::cout << "\n" << pad << "    ";
    for (int c = 0; c < size; ++c)
        std::cout << "---";
    std::cout << "\n";

    for (int r = 0; r < size; ++r) {
        std::cout << pad << std::setw(2) << r << " | ";
        for (int c = 0; c < size; ++c) {
            char ch = board.at(r, c);
            std::cout << "  ";
            if (ch == SYMBOL_X)
                std::cout << CYAN << ch << RESET;
            else if (ch == SYMBOL_O)
                std::cout << YELLOW << ch << RESET;
            else
                std::cout << ch;
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void TerminalRenderer::showMove(int row, int col) {
    if (s_judge_mode) return;
    std::cout << "  -> Move: (" << row << ", " << col << ")\n";
}

void TerminalRenderer::showPlayer(int player, bool is_bot) {
    if (s_judge_mode) return;
    std::cout << "\n--- ";
    if (is_bot) 
        std::cout << "Bot (Player " << player << ") thinking...";
    else 
        std::cout << "Player " << player << "'s turn";
    std::cout << " ---\n";
}

void TerminalRenderer::showInvalidMove() {
    if (s_judge_mode) return;
    std::cout << "Invalid move! Please try again.\n";
}

void TerminalRenderer::showResult(int winner, bool is_bot, const WinLine* winLine) {
    if (winLine && !winLine->cells.empty()) {      
    }
    if (winner == DRAW_RESULT || winner < 0)
        std::cout << RED << "Draw!\n" << RESET;
    else if (is_bot)
        std::cout << GREEN << "Bot (Player " << winner + 1 << ") wins!\n" << RESET;
    else
        std::cout << GREEN << "Player " << winner + 1 << " wins!\n" << RESET;
}

void TerminalRenderer::printResult(const GameResult& gameResult) {
    std::cout << gameResult.winner << " " << gameResult.turns << "\n";
}

void TerminalRenderer::showUndo() {
    if (s_judge_mode) return;
    std::cout << CYAN << "[Undo] Move reverted.\n" << RESET;
}

void TerminalRenderer::close() {
    
}

void TerminalRenderer::showGameInfo(const GameSetup& setup) {
    if (s_judge_mode) return;
    std::cout << "--- Game Info ---\n";
    std::cout << "Size: " << setup.size << "x" << setup.size
              << "  Goal: " << setup.goal
              << "  Mode: " << modeToString((int)setup.mode);
    if (setup.mode == GameMode::PVE)
        std::cout << "  Bot: " << botToString((int)setup.levels[1]);
    else if (setup.mode == GameMode::EVE)
        std::cout << "  Bot1: " << botToString((int)setup.levels[0])
                  << "  Bot2: " << botToString((int)setup.levels[1]);
    std::cout << "  EndRule: " << endRuleToString((int)setup.endRule) << "\n";
}

void TerminalRenderer::showGameResult(const GameResult& result) {
    if (s_judge_mode) return;
    std::cout << "--- Result ---\n";
    std::cout << "Turns: " << result.turns;
    if (result.winner == DRAW_RESULT)
        std::cout << "  Outcome: Draw";
    else
        std::cout << "  Winner: Player " << result.winner + 1
                  << (result.isBot ? " (Bot)" : " (Human)");
    std::cout << "\n";
}
