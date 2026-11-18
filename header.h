#ifndef HEADER_H
#define HEADER_H

#define EMPTY '.'
#define P1 'X'
#define P2 'O'

void printBoard(int rows, int cols, char board[][cols]);
void initBoard(int rows, int cols, char board[][cols]);
void initBoardColsFilled(int rows, int boardColsFilled[rows]);

#endif