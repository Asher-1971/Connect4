#include <assert.h>
#include "header.h"
#include "module3.h"

static int oracle_check_win(const GameState *game, int piece) {
    static const int directions[4][2] = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1}
    };

    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            for (int d = 0; d < 4; d++) {
                int won = 1;
                for (int k = 0; k < game->win_length; k++) {
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
    GameState game = {0};
    game.rows = 6;
    game.cols = 7;
    game.win_length = 4;
    initBoard(&game);

    assert(validateGameState(&game));
    assert(game.current_player == PLAYER_ONE);

    Move move = {0, MOVE_NORMAL};
    GameState first = apply_move(game, move);
    assert(first.board[0][0] == P1);
    assert(first.move_count == 1);
    assert(first.piece_count == 1);
    assert(first.current_player == PLAYER_TWO);

    GameState second = apply_move(first, move);
    assert(second.board[1][0] == P2);
    assert(second.move_count == 2);
    assert(second.piece_count == 2);
    assert(second.current_player == PLAYER_ONE);

    GameState full = second;
    for (int r = 0; r < full.rows; r++) {
        full.board[r][6] = P1;
    }
    full.piece_count = 8;
    assert(!try_apply_move(&full,
                           (Move){6, MOVE_NORMAL}, &full));

    GameState floating = second;
    floating.board[0][0] = EMPTY;
    floating.board[1][0] = P1;
    floating.move_count = 2;
    floating.piece_count = 2;
    floating.current_player = PLAYER_TWO;
    assert(!validateGameState(&floating));

    GameState invalid_cell = second;
    invalid_cell.board[0][0] = 3;
    assert(!validateGameState(&invalid_cell));

    GameState mismatch = second;
    mismatch.move_count = 1;
    mismatch.piece_count = 2;
    assert(!validateGameState(&mismatch));

    GameState horizontal = {0};
    horizontal.rows = 6;
    horizontal.cols = 7;
    horizontal.win_length = 4;
    horizontal.move_count = 4;
    horizontal.piece_count = 4;
    horizontal.current_player = PLAYER_ONE;
    initBoard(&horizontal);
    for (int c = 0; c < 4; c++) {
        horizontal.board[0][c] = P1;
    }
    for (int c = 4; c < 7; c++) {
        horizontal.board[0][c] = P2;
    }
    horizontal.move_count = 7;
    horizontal.piece_count = 7;
    horizontal.current_player = PLAYER_TWO;
    assert(check_winner(&horizontal) == PLAYER_ONE);
    assert(oracle_check_win(&horizontal, P1));

    return 0;
}
