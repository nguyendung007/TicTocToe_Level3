#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <core/types.h>

inline const std::string VERSION = "0.5.25022808";

struct RunConfig {
    bool interactive = true;
    bool judge_mode  = false;
    std::string input_file;

    bool to_file = true;
    std::string log_file = "log.txt";

    bool verbose_flag = false;
    bool gui_flag     = false;

    bool is_help = false;

    std::uint32_t seed = 8702;

    bool parallel_flag = false;
    int  num_threads   = 0;

    int screenWidth  = 800;
    int screenHeight = 800;
    int boardPadding = 100;

    friend std::ostream& operator<<(std::ostream& os, const RunConfig& config);
    std::string toString() const;

    EndRule endRule = EndRule::OPEN_TWO; 
};

RunConfig parseArgs(int argc, char* argv[]);
std::string configHelpStr();
