#pragma once
#include <fstream>
#include <string>

class Logger {
   public:
    enum class Level {
        DEBUG,    
        INFO,     
        WARNING,  
        ERROR,    
        MSG,      
    };  
    Logger(); 
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    void init(bool judge_mode, bool to_file = true,
              const std::string& path = "log.txt", bool verbose_flag = false);
    void log(const std::string& msg, Level level = Level::INFO);
    void close();
    static std::string levelToString(Level level);

   private:
    Level min_level_ = Level::INFO;  
    bool write_to_file_ = false;     
    bool is_judge_mode_ = false;     
    std::ofstream log_file_;         
};
