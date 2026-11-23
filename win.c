#include "header.h"

int countInLine(GameState *game, int r, int c, int dr, int dc, char piece) {
    if (!game || game->rows < 1 || game->rows > MAX_SIZE ||
        game->cols < 1 || game->cols > MAX_SIZE) {
        return 0;
    }
    if (r < 0 || r >= game->rows || c < 0 || c >= game->cols) {
        return 0;
    }
    if (game->board[r][c] != piece) {
        return 0;
    }
    return 1 + countInLine(game, r + dr, c + dc, dr, dc, piece);
}

int checkWin(GameState *game, char piece) {
    if (!game || game->rows < 1 || game->rows > MAX_SIZE ||
        game->cols < 1 || game->cols > MAX_SIZE ||
        (piece != P1 && piece != P2)) {
        return 0;
    }
    int target = (game->winLength >= 4) ? game->winLength : 4;
    if (target > game->rows && target > game->cols) {
        return 0;
    }
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
    if (!game || game->rows < 1 || game->rows > MAX_SIZE ||
        game->cols < 1 || game->cols > MAX_SIZE) {
        return 0;
    }
    if (checkWin(game, P1) || checkWin(game, P2)) {
        return 0;
    }
    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            if (game->board[r][c] == EMPTY) {
                return 0;
            }
        }
    }
    return 1;
}
