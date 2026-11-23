#include "header.h"

int countInLine(GameState *game, int r, int c, int dr, int dc, char piece) {
    if (r < 0 || r >= game->rows || c < 0 || c >= game->cols) {
        return 0;
    }
    if (game->board[r][c] != piece) {
        return 0;
    }
    return 1 + countInLine(game, r + dr, c + dc, dr, dc, piece);
}

int checkWin(GameState *game, char piece) {
    int target = (game->winLength >= 4) ? game->winLength : 4;
    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            if (countInLine(game, r, c, 0, 1, piece) >= target) return 1;   /* Right */
            if (countInLine(game, r, c, 1, 0, piece) >= target) return 1;   /* Down */
            if (countInLine(game, r, c, 1, 1, piece) >= target) return 1;   /* Diagonal down-right */
            if (countInLine(game, r, c, 1, -1, piece) >= target) return 1;  /* Diagonal down-left */
        }
    }
    return 0;
}

int checkTie(GameState *game) {
    return game->moves >= game->rows * game->cols;
}
