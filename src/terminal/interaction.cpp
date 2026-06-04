#include "interaction.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

TerminalInteraction::TerminalInteraction() : I_Interaction() {}
TerminalInteraction::~TerminalInteraction() = default;


static std::streambuf* s_cin_backup = nullptr;
static std::ifstream   s_file_in;


void TerminalInteraction::init(const RunConfig& config) {
    if (!config.interactive && !config.input_file.empty()) {
        s_file_in.open(config.input_file);
        if (s_file_in.is_open()) {
            s_cin_backup = std::cin.rdbuf();
            std::cin.rdbuf(s_file_in.rdbuf());
        }
    }
}


void TerminalInteraction::pause(int /*timeout*/) {
}


static bool getIntInput(int* val) {
    if (!val) return false;
    std::string input;
    if (!std::getline(std::cin, input)) return false;

    auto first = input.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return false;
    auto last = input.find_last_not_of(" \t\n\r");
    input = input.substr(first, last - first + 1);

    for (char c : input)
        if (!std::isdigit((unsigned char)c)) {
            std::cout << "Invalid input! Please enter a valid integer.\n";
            return false;
        }

    try {
        *val = std::stoi(input);
    } catch (...) {
        std::cout << "Invalid input! Please enter a valid integer.\n";
        return false;
    }
    return true;
}

/* ---------- selectSize ---------- */

bool TerminalInteraction::selectSize(int* size) {
    if (!size) return false;
    int temp;
    if (!getIntInput(&temp)) return false;
    if (temp < BOARD_N_MIN || temp > BOARD_N_MAX) {
        std::cout << "Size must be in range [" << BOARD_N_MIN << ", "
                  << BOARD_N_MAX << "]. Please try again.\n";
        return false;
    }
    *size = temp;
    return true;
}

/* ---------- selectGoal ---------- */

bool TerminalInteraction::selectGoal(int* goal, int size) {
    if (!goal) return false;
    int temp;
    if (!getIntInput(&temp)) return false;
    if (temp < 3 || temp > size) {
        std::cout << "Goal must be in range [3, " << size << "]. Please try again.\n";
        return false;
    }
    *goal = temp;
    return true;
}

/* ---------- selectGameMode ---------- */

bool TerminalInteraction::selectGameMode(GameMode* mode) {
    if (!mode) return false;
    int choice;
    if (!getIntInput(&choice)) return false;
    switch (choice) {
        case 1: *mode = GameMode::PVP; break;
        case 2: *mode = GameMode::PVE; break;
        case 3: *mode = GameMode::EVE; break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            return false;
    }
    return true;
}

/* ---------- selectBotLevel ---------- */

bool TerminalInteraction::selectBotLevel(BotLevel* levels, int index) {
    if (!levels) return false;
    int choice;
    if (!getIntInput(&choice)) return false;
    switch (choice) {
        case 1: levels[index] = BotLevel::EASY;   break;
        case 2: levels[index] = BotLevel::MEDIUM; break;
        case 3: levels[index] = BotLevel::HARD;   break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            return false;
    }
    return true;
}

/* ---------- getPlayerMove ---------- */

PlayerInput TerminalInteraction::getPlayerInput() {
    std::string input;
    if (!std::getline(std::cin, input)) return PlayerInput::makeSurrender();

    auto first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return PlayerInput::makeUndo(); 
    input = input.substr(first);

    if (input == "u" || input == "undo")
        return PlayerInput::makeUndo();

    if (input == "s" || input == "surrender")
        return PlayerInput::makeSurrender();

    std::stringstream ss(input);
    int row, col;
    if (ss >> row >> col)
        return PlayerInput::makeMove(Move{row, col});

    return PlayerInput::makeMove(INVALID_MOVE); 
}

bool TerminalInteraction::selectEndRule(EndRule* rule) { 
    int choice;
    if (!getIntInput(&choice)) return false;
    
    switch(choice) {
        case 1: *rule = EndRule::NONE; break;
        case 2: *rule = EndRule::OPEN_ONE; break;
        case 3: *rule = EndRule::OPEN_TWO; break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            return false;
    }
    return true;
}

void TerminalInteraction::close() {
    if (s_cin_backup) {
        std::cin.rdbuf(s_cin_backup);
        s_cin_backup = nullptr;
    }
    if (s_file_in.is_open())
        s_file_in.close();
}
