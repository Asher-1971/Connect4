#ifndef HEADER_H
#define HEADER_H

#define EMPTY '.'
#define P1 'X'
#define P2 'O'
#define MIN_SIZE 4
#define MAX_SIZE 20

// functions for start.c
void printBoard(int rows, int cols, char board[][cols]);
void initBoard(int rows, int cols, char board[][cols]);
void initBoardColsFilled(int rows, int cols, int boardColsFilled[]);
void dropPiece(int cols, char board[][cols], int boardColsFilled[], int colChoice, int *turn);
int readInt(void);
int validateBoardSize(int n);

// fucntions for win.c
int countInLine(int rows, int cols, char board[][cols], int r, int c, int dr, int dc, char piece);
int checkWin(int rows, int cols, char board[][cols], char piece);
int checkTie(int rows, int cols, int moves);

#endif