#ifndef HEADER_H
#define HEADER_H

#include <stdbool.h>
#include <stdint.h>

#define EMPTY 0
#define P1 1
#define P2 2
#define MIN_SIZE 4
#define MAX_SIZE 20
#define MAX_PLAYERS 2

#define PLAYER_NONE 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2

typedef enum {
    MOVE_NORMAL = 0,
    MOVE_BOMB = 1,
    MOVE_ANVIL = 2
} MoveType;

typedef struct {
    int col;
    MoveType type;
} Move;

typedef struct {
    int rows;
    int cols;
    int winLength;
    uint8_t board[MAX_SIZE][MAX_SIZE];
    int current_player;
    int move_count;
    int piece_count;
    bool bomb_used[MAX_PLAYERS + 1];
    bool anvil_used[MAX_PLAYERS + 1];

    /* Terminal UI state retained during migration. */
    int boardColsFilled[MAX_SIZE];
    int choice;
    int aiOrNot;
    int win;
    int tie;
    int colChoice;
} GameState;

/* Functions for start.c */
void printBoard(GameState *game);
void initBoard(GameState *game);
void initBoardColsFilled(GameState *game);
int dropPiece(GameState *game);
int readInt(void);
int validateBoardSize(int n);
int mainstart(void);

/* Functions for win.c */
int countInLine(GameState *game, int r, int c, int dr, int dc, int piece);
int checkWin(GameState *game, int piece);
int checkTie(GameState *game);

#endif
