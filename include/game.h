#ifndef GAME_H
#define GAME_H

#include "header.h"

#define AI_EASY 1
#define AI_NORMAL 2
#define AI_HARD 3

typedef struct {
    int menu_choice;
    int ai_mode;
    int ai_difficulty;
    bool win;
    bool tie;
    int save_slot;   /* slot the game was resumed from, 0 if new */
} GameConfig;

int saveGame(const GameState *game, const GameConfig *config);
int loadGame(GameState *game, GameConfig *config);
void gameInputs(GameState *game, GameConfig *config);
void uiHumanvsHuman(GameState *game, GameConfig *config);
void uiHumanvsAI(GameState *game, GameConfig *config);

#endif
