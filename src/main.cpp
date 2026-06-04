#include <iostream>
#include <memory>
#include "interface/i_interaction.h"
#include "interface/i_renderer.h"
#include "shell/engine.h"
#include "shell/logger.h"
#include "shell/rng.h"
#include "terminal/interaction.h"
#include "terminal/renderer.h"
#include "utils/config.h"

int main(int argc, char* argv[]) {
    
    RunConfig config = parseArgs(argc, argv);

    if (config.is_help) {
        std::cout << configHelpStr();
        return 0;
    }

    Logger logger;
    logger.init(config.judge_mode, config.to_file, config.log_file, config.verbose_flag);
    logger.log("Logger initialized!");
    logger.log(config.toString(), Logger::Level::DEBUG);

    
    Rng rng = makeRng(config.seed);
    logger.log("RNG initialized with seed = " + std::to_string(config.seed));

    std::unique_ptr<I_Renderer> iRenderer;
    std::unique_ptr<I_Interaction> iInteraction;

    if (!config.gui_flag || config.judge_mode) {
        iRenderer = std::make_unique<TerminalRenderer>();
        iInteraction = std::make_unique<TerminalInteraction>();
        logger.log("Terminal renderer & interaction initialized!");
    } else {
        
        logger.log("SDL chưa được kích hoạt trong starter level 3 -- fallback Terminal.",
                   Logger::Level::WARNING);
        iRenderer = std::make_unique<TerminalRenderer>();
        iInteraction = std::make_unique<TerminalInteraction>();
    }

    Engine engine(&config, iRenderer.get(), iInteraction.get(), &logger, &rng);

    try {
        engine.init();
        engine.startGame();
        GameResult result = engine.playGame();
        engine.endGame(result);
    } catch (const QuitException&) {
        logger.log("Quit signal received. Cleaning up...", Logger::Level::WARNING);
    } catch (const NotImplementedException& e) {
        logger.log(std::string("Some functions are not implemented yet: ") + e.what(),
                   Logger::Level::WARNING);
    }

    engine.close();
    logger.log("Engine destroyed!");
    logger.log("Logger closing . . .");
    logger.close();

    return 0;
}





