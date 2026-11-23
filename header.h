#ifndef HEADER_H
#define HEADER_H

#define EMPTY '.'
#define P1 'X'
#define P2 'O'
#define MIN_SIZE 4
#define MAX_SIZE 20

typedef struct {
    int rows;
    int cols;
<<<<<<< HEAD
=======
    int winLength;
>>>>>>> origin/module3
    char board[MAX_SIZE][MAX_SIZE];
    int boardColsFilled[MAX_SIZE];
    int choice;
    int aiOrNot;
    int win;
    int tie;
    int moves;
    int colChoice;
    int turn;
} GameState;

<<<<<<< HEAD
// functions for start.c
void printBoard(GameState *game);
void initBoard(GameState *game);
void initBoardColsFilled(GameState *game);
void dropPiece(GameState *game);
int readInt(void);
int validateBoardSize(int n);

// fucntions for win.c
=======
/* Functions for start.c */
void printBoard(GameState *game);
void initBoard(GameState *game);
void initBoardColsFilled(GameState *game);
int dropPiece(GameState *game);
int readInt(void);
int validateBoardSize(int n);
int mainstart(void);

/* Functions for win.c */
>>>>>>> origin/module3
int countInLine(GameState *game, int r, int c, int dr, int dc, char piece);
int checkWin(GameState *game, char piece);
int checkTie(GameState *game);

#endif