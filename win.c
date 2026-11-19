#include <stdio.h>
#include "header.h"

#include "header.h"

// counts how many of 'piece' appear in a row starting at (r, c),
// moving by (dr, dc) each step
int countInLine(int rows, int cols, char board[][cols], int r, int c, int dr, int dc, char piece){
    // base case: off the board
    if (r < 0 || r >= rows || c < 0 || c >= cols) {
        return 0;
    }
    // base case: not our piece
    if (board[r][c] != piece) {
        return 0;
    }
    // recursive case: this cell counts as 1, plus the rest of the line
    return 1 + countInLine(rows, cols, board, r + dr, c + dc, dr, dc, piece);
}

int checkWin(int rows, int cols, char board[][cols], char piece){
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (countInLine(rows, cols, board, r, c, 0, 1, piece) >= 4) return 1;   // right
            if (countInLine(rows, cols, board, r, c, 1, 0, piece) >= 4) return 1;   // down
            if (countInLine(rows, cols, board, r, c, 1, 1, piece) >= 4) return 1;   // diagonal down-right
            if (countInLine(rows, cols, board, r, c, 1, -1, piece) >= 4) return 1;  // diagonal down-left
        }
    }
    return 0;
}