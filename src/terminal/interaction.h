#pragma once
#include "../interface/i_interaction.h"

class TerminalInteraction : public I_Interaction {
   public:
    TerminalInteraction();
    ~TerminalInteraction() override;

    void init(const RunConfig& config) override;
    void pause(int timeout = 0) override;

    bool selectSize(int* size) override;
    bool selectGoal(int* goal, int size) override;
    bool selectGameMode(GameMode* mode) override;
    bool selectBotLevel(BotLevel* levels, int index) override;

    PlayerInput getPlayerInput() override;

    void close() override;

    bool selectEndRule(EndRule* rule) override;

};
