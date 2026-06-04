#pragma once
#include "../core/bot_pure.h"
#include "../core/types.h"
#include "../interface/i_interaction.h"
#include "../interface/i_renderer.h"
#include "../utils/config.h"
#include "logger.h"
#include "rng.h"
#include <vector>

struct RenderCmd;  \

class Engine {
   public:
    Engine(const RunConfig* config,
           I_Renderer* iRenderer,
           I_Interaction* iInteraction,
           Logger* logger,
           Rng* rng);

    ~Engine();

    void init();
    void startGame();
    GameResult playGame();
    void endGame(const GameResult& gameResult);
    void close();

   private:
    const RunConfig* config_;
    I_Renderer* iRenderer_;
    I_Interaction* iInteraction_;
    Logger* logger_;   
    Rng* rng_;         

    GameSetup gameSetup_;
    GameState gameState_;  

    bool sanity_check();
    GameResult loopGame(GameState state,
                    std::vector<GameState> history,
                    bot::BotFn bot0, bot::BotFn bot1);
    void executeRenderCmd(const RenderCmd& cmd);
};
