#pragma once
#include <array>
#include <exception>
#include <string>
#include <vector>

inline constexpr int BOARD_N_MIN = 3;    
inline constexpr int BOARD_N_MAX = 12;   
inline constexpr int GOAL_MAX = 5;       
inline constexpr int SLEEP_TIME = 1500;  

inline constexpr int SCORE_INF = 1000;     
inline constexpr int EVALUATE_SCORE = 10;  

inline constexpr char EMPTY_CELL = '.';  
inline constexpr char SYMBOL_X = 'X';    
inline constexpr char SYMBOL_O = 'O';    

inline constexpr int DRAW_RESULT = -1;  
inline constexpr int NO_CONTEXT = -1;   

enum class BotLevel {EASY, MEDIUM, HARD, INVALID_LV};

enum class GameMode {PVP, PVE, EVE, INVALID_MODE};

inline std::string botToString(int v) {
    switch ((BotLevel)v) {
        case BotLevel::EASY:   return "EASY";
        case BotLevel::MEDIUM: return "MEDIUM";
        case BotLevel::HARD:   return "HARD";
        default:               return "?";
    }
}

inline std::string modeToString(int v) {
    switch ((GameMode)v) {
        case GameMode::PVP: return "PVP";
        case GameMode::PVE: return "PVE";
        case GameMode::EVE: return "EVE";
        default:            return "?";
    }
}

inline static std::string endRuleToString(int rule) {
    switch (rule) {
        case 0: return "None";
        case 1: return "OpenOne";
        case 2: return "OpenTwo";
        default: return "Unknown";
    }
}


struct Move {
    int row;
    int col;

    constexpr bool operator==(const Move& o) const noexcept {
        return row == o.row && col == o.col;
    }
};

inline constexpr Move INVALID_MOVE{-1, -1};

using pII = std::pair<int, int>;

enum class SelectType {
    TITLE_UI,
    SIZE_UI,
    GOAL_UI,
    ENDRULE_UI,
    GAME_MODE_UI,
    BOT_LEVEL_UI,
    PLAYER_UI,
    MUL_BOT_LEVEL_UI,
    INVALID_UI
};

enum class EndRule {NONE, OPEN_ONE, OPEN_TWO   
};

enum class InputKind { Move, Undo, Surrender };

struct Board {
    std::array<std::array<char, BOARD_N_MAX>, BOARD_N_MAX> grid{};
    int size{0};

    constexpr char at(int r, int c) const noexcept {
        return grid[r][c];
    }

    constexpr bool inRange(int r, int c) const noexcept {
        return 0 <= r && r < size && 0 <= c && c < size;
    }

    bool operator==(const Board& o) const noexcept {
        if (size != o.size) return false;
        for (int r = 0; r < size; ++r)
            for (int c = 0; c < size; ++c)
                if (grid[r][c] != o.grid[r][c]) return false;
        return true;
    }
};

struct GameState {
    Board board;          
    int currentPlayer;    
    int turn;             
    int winner;           
    bool isFinished;      
};

struct GameSetup {
    int size{0};                 
    int goal{0};                 
    GameMode mode{GameMode::INVALID_MODE};
    std::array<BotLevel, 2> levels{BotLevel::INVALID_LV, BotLevel::INVALID_LV};
    EndRule endRule = EndRule::OPEN_TWO;  
};

struct GameResult {
    int winner;   
    bool isBot;   
    int turns;    

    constexpr GameResult(int _winner, bool _isBot, int _turns) noexcept
        : winner(_winner), isBot(_isBot), turns(_turns) {}
};

struct WinLine {
    std::vector<pII> cells;
};

struct PlayerInput {
    InputKind kind;
    Move move{INVALID_MOVE};   

    static constexpr PlayerInput makeMove(Move m) noexcept {
        return PlayerInput{InputKind::Move, m};
    }
    static constexpr PlayerInput makeUndo() noexcept {
        return PlayerInput{InputKind::Undo, INVALID_MOVE};
    }
    static constexpr PlayerInput makeSurrender() noexcept {
        return PlayerInput{InputKind::Surrender, INVALID_MOVE};
    }
};

class QuitException : public std::exception {
   public:
    const char* what() const noexcept override {
        return "User requested quit";
    }
};

class NotImplementedException : public std::exception {
   public:
    const char* what() const noexcept override {
        return "Functionality not implemented yet";
    }
};

