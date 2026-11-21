#ifndef HEADER_H
#define HEADER_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define NAME_LEN 20
#define MAX_SIZE 20
#define SAVE_FILE  "save_game.dat"

#define TWO_PLAYER 1
#define VS_COMPUTER 2
#define EASY 1
#define MEDIUM 2

extern int **board;
extern int rows,cols,winLen;
extern int mode;
extern int level;
extern int current;
extern int bombs[3], anvils[3];
extern char playerName[3][NAME_LEN+1];
extern char symbol[3];

void createBoard(int r, int c);
void freeBoard(void);
void printBoard(void);
int dropDisc(int col, int player);
int useBomb(int col, int player);
int useAnvil(int col, int player);
int undoMove(void);


int isBoardFull(void);

int getAiMove(int lvl, int player);

//my functions
void mainMenu(void);
void playgame(void);
void saveGame(void);
void loadGame(void);


#endif