
#ifndef GAME_H
#define GAME_H
 
#include "header.h"   /* GameState and the start.c / win.c functions */
 
/* Screen helpers */
void wait_for_enter(void);
void clearScreen(void);
 
/* Menu and game loops */
void gameInputs(GameState *game);
void uiHumanvsHuman(GameState *game);
void uiHumanvsAI(GameState *game);
 
/* Save / load (called from the UI, not declared in header.h yet) */
int saveGame(GameState *game);
int loadGame(GameState *game);
 
#endif