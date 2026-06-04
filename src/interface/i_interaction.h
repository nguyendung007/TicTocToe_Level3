#pragma once
#include "../core/types.h"
#include "../utils/config.h"

class I_Interaction {
   public:
    I_Interaction();
    virtual ~I_Interaction();

    virtual void init(const RunConfig& config) = 0;
    virtual void pause(int timeout = 0) = 0;

    virtual bool selectSize(int* size) = 0;
    virtual bool selectGoal(int* goal, int size) = 0;
    virtual bool selectGameMode(GameMode* mode) = 0;
    virtual bool selectBotLevel(BotLevel* levels, int index) = 0;

    virtual PlayerInput getPlayerInput() = 0;
    virtual void close() = 0;

    virtual bool selectEndRule(EndRule* rule) = 0; 
    
};
