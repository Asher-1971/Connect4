
#include "header.h"

// counts how many of 'piece' appear in a row starting at (r, c),
// moving by (dr, dc) each step
int countInLine(GameState *game, int r, int c, int dr, int dc, char piece){
    // base case: off the board
    if (r < 0 || r >= game->rows || c < 0 || c >= game->cols) {
        return 0;
    }
    // base case: not our piece
    if (game->board[r][c] != piece) {
        return 0;
    }
    // recursive case: this cell counts as 1, plus the rest of the line
    return 1 + countInLine(game, r + dr, c + dc, dr, dc, piece);
}

int checkWin(GameState *game, char piece){
    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            if (countInLine(game, r, c, 0, 1, piece) >= 4) return 1;   // right
            if (countInLine(game, r, c, 1, 0, piece) >= 4) return 1;   // down
            if (countInLine(game, r, c, 1, 1, piece) >= 4) return 1;   // diagonal down-right
            if (countInLine(game, r, c, 1, -1, piece) >= 4) return 1;  // diagonal down-left
        }
    }
    return 0;
}

int checkTie(GameState *game){
    return game->moves >= game->rows * game->cols;
}