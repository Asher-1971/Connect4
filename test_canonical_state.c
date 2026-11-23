#include <assert.h>
#include "header.h"
#include "module3.h"

static int oracle_check_win(const GameState *game, int piece) {
    static const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            for (int d = 0; d < 4; d++) {
                int won = 1;
                for (int k = 0; k < game->winLength; k++) {
                    int rr = r + k * directions[d][0];
                    int cc = c + k * directions[d][1];
                    if (rr < 0 || rr >= game->rows ||
                        cc < 0 || cc >= game->cols ||
                        game->board[rr][cc] != piece) {
                        won = 0;
                        break;
                    }
                }
                if (won) return 1;
            }
        }
    }
    return 0;
}

int main(void) {
    assert(!dropPiece(NULL));

    GameState invalid_dimensions = {0};
    invalid_dimensions.rows = 21;
    invalid_dimensions.cols = 7;
    initBoard(&invalid_dimensions);
    assert(invalid_dimensions.piece_count == 0);
    assert(invalid_dimensions.move_count == 0);
    invalid_dimensions.rows = 6;
    invalid_dimensions.cols = 7;
    invalid_dimensions.winLength = 4;
    invalid_dimensions.rows = 21;
    invalid_dimensions.colChoice = 1;
    assert(!dropPiece(&invalid_dimensions));

    GameState game = {0};
    game.rows = 6;
    game.cols = 7;
    game.winLength = 4;
    initBoard(&game);

    assert(game.current_player == PLAYER_ONE);
    assert(game.move_count == 0);
    assert(game.piece_count == 0);
    assert(game.bomb_used[PLAYER_ONE] == false);
    assert(game.anvil_used[PLAYER_TWO] == false);

    assert(!dropPiece(NULL));
    game.colChoice = 0;
    assert(!dropPiece(&game));
    game.colChoice = 8;
    assert(!dropPiece(&game));

    game.colChoice = 1;
    assert(dropPiece(&game));
    assert(game.board[0][0] == P1);
    assert(game.move_count == 1);
    assert(game.piece_count == 1);
    assert(game.current_player == PLAYER_TWO);

    game.colChoice = 1;
    assert(dropPiece(&game));
    assert(game.board[1][0] == P2);
    assert(game.move_count == 2);
    assert(game.piece_count == 2);
    assert(game.current_player == PLAYER_ONE);

    for (int r = 0; r < game.rows; r++) {
        game.board[r][6] = P1;
    }
    game.piece_count = 8;
    game.move_count = 8;
    game.colChoice = 7;
    assert(!dropPiece(&game));
    assert(game.move_count == 8);
    assert(game.piece_count == 8);

    GameState mismatch = game;
    mismatch.move_count = 7;
    mismatch.piece_count = 8;
    assert(getBestMove(&mismatch, 4, 1) == -1);

    GameState invalid_cell = game;
    invalid_cell.board[0][0] = 3;
    assert(getBestMove(&invalid_cell, 4, 1) == -1);

    GameState valid = {0};
    valid.rows = 6;
    valid.cols = 7;
    valid.winLength = 4;
    initBoard(&valid);
    valid.colChoice = 1;
    assert(dropPiece(&valid));
    valid.colChoice = 1;
    assert(dropPiece(&valid));
    assert(validateGameState(&valid));

    mismatch = valid;
    mismatch.move_count = 1;
    mismatch.piece_count = 8;
    assert(!validateGameState(&mismatch));

    GameState fresh = {0};
    fresh.rows = 6;
    fresh.cols = 7;
    fresh.winLength = 4;
    initBoard(&fresh);
    Move move = {3, MOVE_NORMAL};
    GameState applied = apply_move(fresh, move);
    assert(applied.board[0][3] == P1);
    assert(applied.move_count == 1);
    assert(applied.piece_count == 1);
    assert(applied.current_player == PLAYER_TWO);

    GameState horizontal = {0};
    horizontal.rows = 6;
    horizontal.cols = 7;
    horizontal.winLength = 4;
    horizontal.move_count = 4;
    horizontal.piece_count = 4;
    horizontal.current_player = PLAYER_ONE;
    initBoard(&horizontal);
    horizontal.move_count = 4;
    horizontal.piece_count = 4;
    for (int c = 0; c < 4; c++) horizontal.board[0][c] = P1;
    assert(checkWin(&horizontal, P1));
    assert(oracle_check_win(&horizontal, P1));

    GameState vertical = horizontal;
    initBoard(&vertical);
    vertical.move_count = 4;
    vertical.piece_count = 4;
    for (int r = 0; r < 4; r++) vertical.board[r][0] = P2;
    assert(checkWin(&vertical, P2));
    assert(oracle_check_win(&vertical, P2));

    GameState diagonal = horizontal;
    initBoard(&diagonal);
    diagonal.move_count = 4;
    diagonal.piece_count = 4;
    for (int k = 0; k < 4; k++) diagonal.board[k][k] = P1;
    assert(checkWin(&diagonal, P1));
    assert(oracle_check_win(&diagonal, P1));

    GameState anti_diagonal = horizontal;
    initBoard(&anti_diagonal);
    anti_diagonal.move_count = 4;
    anti_diagonal.piece_count = 4;
    for (int k = 0; k < 4; k++) anti_diagonal.board[k][3 - k] = P2;
    assert(checkWin(&anti_diagonal, P2));
    assert(oracle_check_win(&anti_diagonal, P2));

    return 0;
}
