#ifndef GAME_H
#define GAME_H

#include "header.h"

#define AI_EASY 1
#define AI_NORMAL 2
#define AI_HARD 3

void wait_for_enter(void);
void clearScreen(void);
int saveGame(GameState *game);
int loadGame(GameState *game);
void gameInputs(GameState *game);
void uiHumanvsHuman(GameState *game);
void uiHumanvsAI(GameState *game);

#endif
