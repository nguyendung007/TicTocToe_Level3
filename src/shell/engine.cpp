#include "engine.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include "../core/bot_pure.h"
#include "../core/logic.h"
#include "../utils/helper.h"

Engine::Engine(const RunConfig* config,
               I_Renderer* iRenderer,
               I_Interaction* iInteraction,
               Logger* logger,
               Rng* rng)
    : config_(config),
      iRenderer_(iRenderer),
      iInteraction_(iInteraction),
      logger_(logger),
      rng_(rng) {}

Engine::~Engine() = default;

void Engine::init() {
    if (logger_) logger_->log("Engine initializing . . .");
    if (iRenderer_)    iRenderer_->init(*config_);
    if (iInteraction_) iInteraction_->init(*config_);
    if (logger_) logger_->log("Engine initialized!");
}

bool Engine::sanity_check() {
    bool ok = true;
    if (!iRenderer_) {
        if (logger_) logger_->log("Renderer not implemented!", Logger::Level::WARNING);
        ok = false;
    }
    if (!iInteraction_) {
        if (logger_) logger_->log("Interaction not implemented!", Logger::Level::WARNING);
        ok = false;
    }
    if (!logger_) ok = false;
    if (!rng_) {
        if (logger_) logger_->log("RNG not provided!", Logger::Level::ERROR);
        ok = false;
    }
    return ok;
}

void Engine::close() {
    if (logger_) logger_->log("Engine closing . . .");
    if (iRenderer_)    iRenderer_->close();
    if (iInteraction_) iInteraction_->close();
    if (logger_) logger_->log("Engine closed!");
}

void Engine::startGame() {
    if (logger_) logger_->log("[Engine] Starting game . . .");
    if (!sanity_check()) {
        if (logger_) logger_->log("[Engine] Game stopped!", Logger::Level::ERROR);
        return;
    }

    if (config_->interactive) {
        iRenderer_->clearScreen();
        iRenderer_->showSelectMenu(SelectType::TITLE_UI);
    }

    do {
        if (config_->interactive)
            iRenderer_->showSelectMenu(SelectType::SIZE_UI);
    } while (!iInteraction_->selectSize(&gameSetup_.size));

    if (logger_) logger_->log("[Engine] Board size = " + std::to_string(gameSetup_.size));

    do {
        if (config_->interactive)
            iRenderer_->showSelectMenu(SelectType::GOAL_UI, gameSetup_.size);
    } while (!iInteraction_->selectGoal(&gameSetup_.goal, gameSetup_.size));

    if (logger_) logger_->log("[Engine] Goal = " + std::to_string(gameSetup_.goal));

    do {
    if (config_->interactive)
        iRenderer_->showSelectMenu(SelectType::ENDRULE_UI, gameSetup_.goal);  
    } while (!iInteraction_->selectEndRule(&gameSetup_.endRule));

    if (logger_) {
    std::string ruleStr;
    switch (gameSetup_.endRule) {
        case EndRule::NONE: ruleStr = "NONE"; break;
        case EndRule::OPEN_ONE: ruleStr = "OPEN_ONE"; break;
        case EndRule::OPEN_TWO: ruleStr = "OPEN_TWO"; break;
    }
    logger_->log("[Engine] EndRule = " + ruleStr);
}

    do {
        if (config_->interactive)
            iRenderer_->showSelectMenu(SelectType::GAME_MODE_UI);
    } while (!iInteraction_->selectGameMode(&gameSetup_.mode));

    if (logger_) logger_->log("[Engine] Mode = " + modeToString((int)gameSetup_.mode));

    if (gameSetup_.mode == GameMode::PVP) {
        gameSetup_.levels[0] = BotLevel::INVALID_LV;
        gameSetup_.levels[1] = BotLevel::INVALID_LV;
    } else if (gameSetup_.mode == GameMode::PVE) {
        gameSetup_.levels[0] = BotLevel::INVALID_LV;
        if (config_->interactive)
            iRenderer_->showSelectMenu(SelectType::BOT_LEVEL_UI);
        do {
            if (!config_->interactive)
                iRenderer_->showSelectMenu(SelectType::BOT_LEVEL_UI);
        } while (!iInteraction_->selectBotLevel(gameSetup_.levels.data(), 1));
        if (logger_)
            logger_->log("[Engine] Bot level = " + botToString((int)gameSetup_.levels[1]));
    } else {
        if (config_->interactive)
            iRenderer_->showSelectMenu(SelectType::MUL_BOT_LEVEL_UI);
        do {
            if (!config_->interactive)
                iRenderer_->showSelectMenu(SelectType::BOT_LEVEL_UI);
        } while (!iInteraction_->selectBotLevel(gameSetup_.levels.data(), 0));
        do {
            if (!config_->interactive)
                iRenderer_->showSelectMenu(SelectType::BOT_LEVEL_UI);
        } while (!iInteraction_->selectBotLevel(gameSetup_.levels.data(), 1));
        if (logger_) {
            logger_->log("[Engine] Bot1 = " + botToString((int)gameSetup_.levels[0]));
            logger_->log("[Engine] Bot2 = " + botToString((int)gameSetup_.levels[1]));
        }
    }

    gameState_.board         = core::initBoard(gameSetup_.size);
    gameState_.currentPlayer = 0;
    gameState_.turn          = 0;
    gameState_.winner        = DRAW_RESULT;
    gameState_.isFinished    = false;

    if (logger_)
        logger_->log("[Engine] Board initialized, size=" + std::to_string(gameSetup_.size)
                     + " goal=" + std::to_string(gameSetup_.goal));
    
    if (!config_->judge_mode && iRenderer_) {
        iRenderer_->showGameInfo(gameSetup_);
}
}

static bot::BotFn getBotFn(const GameSetup& setup, int playerIdx) {
    BotLevel lv = setup.levels[playerIdx];
    if (lv == BotLevel::INVALID_LV) return {};
    return bot::makeBot(lv, setup.goal, setup.endRule);
}

static int botSleepMs(BotLevel level) {
    switch (level) {
        case BotLevel::EASY:   return 800;
        case BotLevel::MEDIUM: return 1200;
        case BotLevel::HARD:   return 1500;
        default:               return 800;
    }
}


//helper
// ── Functional Core ───────────────────────────────────────────────────────────

enum class TurnEventKind { BotMove, PlayerMove, Undo, Surrender, InvalidMove };

struct RenderCmd {
    bool clearScreen   = false;
    bool showPlayer    = false;
    bool displayBoard  = false;
    bool showMoveCoord = false;
    bool showUndo      = false;
    bool showInvalid   = false;
    int  playerNum     = 0;
    bool playerIsBot   = false;
    Move move          = INVALID_MOVE;
};

struct TurnResult {
    TurnEventKind kind;
    GameState     nextState;
    std::vector<GameState> nextHistory;
    RenderCmd     cmd;
    std::string   logMsg;
};

static RenderCmd makeShowTurnCmd(int playerNum, bool isBot, bool judgeMode) {
    if (judgeMode) return {};
    RenderCmd c;
    c.clearScreen  = true;
    c.showPlayer   = true;
    c.displayBoard = true;
    c.playerNum    = playerNum;
    c.playerIsBot  = isBot;
    return c;
}

static TurnResult processBotTurn(const GameState& state,
                                 const std::vector<GameState>& history,
                                 Move move,
                                 int goal, int cp, bool judgeMode, EndRule rule) {
    auto newHistory = [&] {
        auto h = history;
        h.push_back(state);
        return h;
    }();

    GameState nextState = core::applyTurn(state, move, goal, rule);

    RenderCmd cmd    = makeShowTurnCmd(cp + 1, true, judgeMode);
    cmd.showMoveCoord = !judgeMode;
    cmd.move          = move;

    return TurnResult{
        TurnEventKind::BotMove,
        nextState,
        std::move(newHistory),
        cmd,
        "[Engine] Bot move: (" + std::to_string(move.row)
          + "," + std::to_string(move.col) + ")"
    };
}

static TurnResult processSurrender(const GameState& state,
                                   const std::vector<GameState>& history,
                                   int cp) {
    return TurnResult{
        TurnEventKind::Surrender,
        core::surrender(state),
        history,
        {},
        "[Engine] Player " + std::to_string(cp + 1) + " surrendered."
    };
}

static TurnResult processUndo(const GameState& state,
                              const std::vector<GameState>& history) {
    if (history.empty()) {
        RenderCmd cmd;
        cmd.showInvalid = false; 
        return TurnResult{TurnEventKind::InvalidMove, state, history, {}, "undo_empty"};
    }
    GameState prev = history.back();
    auto shorterHistory = std::vector<GameState>(history.begin(), history.end() - 1);
    RenderCmd cmd;
    cmd.showUndo = true;
    return TurnResult{TurnEventKind::Undo, prev, std::move(shorterHistory), cmd, ""};
}

static TurnResult processPlayerMove(const GameState& state,
                                    const std::vector<GameState>& history,
                                    Move candidate,
                                    int goal, int cp, bool judgeMode, EndRule rule) {
    if (!core::isValidMove(state.board, candidate)) {
        RenderCmd cmd;
        cmd.showInvalid = true;
        return TurnResult{TurnEventKind::InvalidMove, state, history, cmd, ""};
    }

    auto newHistory = [&] {
        auto h = history;
        h.push_back(state);
        return h;
    }();

    GameState nextState = core::applyTurn(state, candidate, goal, rule);

    RenderCmd cmd    = makeShowTurnCmd(cp + 1, false, judgeMode);
    cmd.showMoveCoord = !judgeMode;
    cmd.move          = candidate;

    return TurnResult{
        TurnEventKind::PlayerMove,
        nextState,
        std::move(newHistory),
        cmd,
        "[Engine] Player " + std::to_string(cp + 1)
          + " move: (" + std::to_string(candidate.row)
          + "," + std::to_string(candidate.col) + ")"
    };
}

// ── Imperative Shell ──────────────────────────────────────────────────────────

void Engine::executeRenderCmd(const RenderCmd& cmd) {
    if (cmd.clearScreen)   iRenderer_->clearScreen();
    if (cmd.showPlayer)    iRenderer_->showPlayer(cmd.playerNum, cmd.playerIsBot);
    if (cmd.displayBoard)  iRenderer_->displayBoard(gameState_.board); 
    if (cmd.showMoveCoord) iRenderer_->showMove(cmd.move.row, cmd.move.col);
    if (cmd.showUndo)      iRenderer_->showUndo();
    if (cmd.showInvalid)   iRenderer_->showInvalidMove();
}

// ── loopGame ─────────────────────────────────────────────────────────────────
//TCO 
GameResult Engine::loopGame(GameState state,
                             std::vector<GameState> history,
                             bot::BotFn bot0, bot::BotFn bot1) {
    // ── Base case ─────────────────────────────────────────────────
    if (state.isFinished) {
        int w     = state.winner;
        bool isBot = (gameSetup_.levels[w == DRAW_RESULT ? 0 : w] != BotLevel::INVALID_LV);
        return GameResult{w, isBot, state.turn};
    }

    int  cp    = state.currentPlayer;
    bool isBot = (gameSetup_.levels[cp] != BotLevel::INVALID_LV);

    // ── Imperative Shell: hiển thị đầu lượt ─────────────────────
    if (!config_->judge_mode) {
        iRenderer_->clearScreen();
        iRenderer_->showGameInfo(gameSetup_);         
        iRenderer_->showPlayer(cp + 1, isBot);
        iRenderer_->displayBoard(state.board);
    }

    // ── Bot turn ──────────────────────────────────────────────────
    if (isBot) {
        auto logSink = [this](const std::string& msg) {
            if (logger_) logger_->log(msg, Logger::Level::DEBUG);
        };

        // Shell: đo thời gian (side effect: log)
        Move move = measureExecutionTime(
            "botMove",
            [&]{ return (cp == 0 ? bot0 : bot1)(state, *rng_); },
            config_->verbose_flag,
            logSink
        );

        // Shell: delay (side effect)
        std::this_thread::sleep_for(
            std::chrono::milliseconds(botSleepMs(gameSetup_.levels[cp])));

        // Core: tính toán thuần
        TurnResult tr = processBotTurn(state, history, move,
                                       gameSetup_.goal, cp, config_->judge_mode, gameSetup_.endRule);

        // Shell: thực thi render commands
        if (!config_->judge_mode) executeRenderCmd(tr.cmd);
        if (logger_ && !tr.logMsg.empty())
            logger_->log(tr.logMsg, Logger::Level::DEBUG);

        // Shell: cập nhật snapshot cho endGame (giữ lại hành vi gốc)
        gameState_ = tr.nextState;

        return loopGame(tr.nextState, std::move(tr.nextHistory), bot0, bot1);
    }

    // ── Player turn ───────────────────────────────────────────────
    while (true) {
        // Shell: lấy input (side effect)
        iRenderer_->showSelectMenu(SelectType::PLAYER_UI);
        PlayerInput input = iInteraction_->getPlayerInput();

        TurnResult tr;

        if (input.kind == InputKind::Surrender) {
            // Core
            tr = processSurrender(state, history, cp);
            // Shell
            if (logger_) logger_->log(tr.logMsg, Logger::Level::DEBUG);
            gameState_ = tr.nextState;
            return loopGame(tr.nextState, std::move(tr.nextHistory), bot0, bot1);
        }

        if (input.kind == InputKind::Undo) {
            // Core
            tr = processUndo(state, history);
            if (tr.logMsg == "undo_empty") {
                // Shell: không có gì để undo
                std::cout << "Nothing to undo!\n";
                continue;
            }
            // Shell
            executeRenderCmd(tr.cmd);
            gameState_ = tr.nextState;
            return loopGame(tr.nextState, std::move(tr.nextHistory), bot0, bot1);
        }

        // InputKind::Move
        tr = processPlayerMove(state, history, input.move,
                               gameSetup_.goal, cp, config_->judge_mode, gameSetup_.endRule);

        if (tr.kind == TurnEventKind::InvalidMove) {
            // Shell
            executeRenderCmd(tr.cmd);
            continue;
        }

        // Shell
        if (!config_->judge_mode) executeRenderCmd(tr.cmd);
        if (logger_) logger_->log(tr.logMsg, Logger::Level::DEBUG);
        gameState_ = tr.nextState;
        return loopGame(tr.nextState, std::move(tr.nextHistory), bot0, bot1);
    }
}

// ── playGame ─────────────────────────────────────────────────────────────────

GameResult Engine::playGame() {
    if (logger_) logger_->log("[Engine] Playing game . . .");

    GameResult fallback{DRAW_RESULT, false, 0};
    if (!sanity_check()) {
        if (logger_) logger_->log("[Engine] Game stopped!", Logger::Level::ERROR);
        return fallback;
    }

    bot::BotFn bot0 = getBotFn(gameSetup_, 0);
    bot::BotFn bot1 = getBotFn(gameSetup_, 1);

    return loopGame(gameState_, {}, bot0, bot1);
}

// ── endGame ──────────────────────────────────────────────────────────────────

void Engine::endGame(const GameResult& gameResult) {
    if (logger_) logger_->log("[Engine] Ending game . . .");
    if (!sanity_check()) {
        if (logger_) logger_->log("[Engine] Game stopped!", Logger::Level::ERROR);
        return;
    }

    if (config_->judge_mode) {
        iRenderer_->printResult(gameResult);
    } else {
        iRenderer_->clearScreen();

        if (gameResult.winner != DRAW_RESULT) {
            char winSym     = core::symbolOf(gameResult.winner);
            auto winLineOpt = core::getWinLine(gameState_.board, winSym, gameSetup_.goal);
            if (winLineOpt.has_value()) {
                iRenderer_->showResult(gameResult.winner, gameResult.isBot,
                                       &winLineOpt.value());
            } else {
                iRenderer_->displayBoard(gameState_.board);
                iRenderer_->showResult(gameResult.winner, gameResult.isBot);
            }
        } else {
            iRenderer_->displayBoard(gameState_.board);
            iRenderer_->showResult(gameResult.winner, gameResult.isBot);
        }
    }
    if (!config_->judge_mode && iRenderer_) {
        iRenderer_->showGameResult(gameResult);
    }

    if (logger_) {
        std::string resultStr;
        if (gameResult.winner == DRAW_RESULT) resultStr = "Draw";
        else if (gameResult.isBot)            resultStr = "Bot (Player " + std::to_string(gameResult.winner + 1) + ") wins";
        else                                  resultStr = "Player " + std::to_string(gameResult.winner + 1) + " wins";
        logger_->log("[Engine] Result: " + resultStr
                     + " | Turns: " + std::to_string(gameResult.turns));
    }
}