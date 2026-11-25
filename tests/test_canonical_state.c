#include <assert.h>
#include "header.h"
#include "module3.h"
#include "rules.h"
#include "terminal.h"

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
    terminal_init_board(&game);

    assert(rules_validate_state(&game));
    assert(game.current_player == PLAYER_ONE);

    Move move = {0, MOVE_NORMAL};
    GameState first = {0};
    assert(rules_apply_move(&game, move, &first));
    assert(first.board[0][0] == P1);
    assert(first.move_count == 1);
    assert(first.piece_count == 1);
    assert(first.current_player == PLAYER_TWO);

    GameState second = {0};
    assert(rules_apply_move(&first, move, &second));
    assert(second.board[1][0] == P2);
    assert(second.move_count == 2);
    assert(second.piece_count == 2);
    assert(second.current_player == PLAYER_ONE);

    GameState full = second;
    for (int r = 0; r < full.rows; r++) {
        full.board[r][6] = P1;
    }
    full.piece_count = 8;
    assert(!rules_apply_move(&full,
                             (Move){6, MOVE_NORMAL}, &full));
    assert(!rules_is_valid_move(&full, (Move){6, MOVE_NORMAL}));

    GameState floating = second;
    floating.board[0][0] = EMPTY;
    floating.board[1][0] = P1;
    floating.move_count = 2;
    floating.piece_count = 2;
    floating.current_player = PLAYER_TWO;
    assert(!rules_validate_state(&floating));

    GameState invalid_cell = second;
    invalid_cell.board[0][0] = 3;
    assert(!rules_validate_state(&invalid_cell));

    GameState mismatch = second;
    mismatch.move_count = 1;
    mismatch.piece_count = 2;
    assert(!rules_validate_state(&mismatch));

    GameState powerup_state = second;
    powerup_state.move_count = 3;
    powerup_state.piece_count = 1;
    powerup_state.board[0][0] = P1;
    for (int r = 1; r < powerup_state.rows; r++) {
        for (int c = 0; c < powerup_state.cols; c++) {
            powerup_state.board[r][c] = EMPTY;
        }
    }
    for (int c = 1; c < powerup_state.cols; c++) {
        powerup_state.board[0][c] = EMPTY;
    }
    uint8_t draw_board[4][4] = {
        {P1, P1, P2, P2},
        {P2, P2, P1, P1},
        {P1, P1, P2, P2},
        {P2, P2, P1, P1}
    };
    /* Classic rules: 3 moves but 1 piece is impossible, and bombs/anvils
     * are refused. */
    assert(!rules_validate_state(&powerup_state));
    assert(!rules_is_valid_move(&game, (Move){0, MOVE_BOMB}));
    assert(!rules_is_valid_move(&game, (Move){0, MOVE_ANVIL}));

    /* Power-ups on. */
    powerup_state.powerups = true;
    Move anvil_move = {6, MOVE_ANVIL};
    powerup_state.move_count = 1;
    powerup_state.piece_count = 1;
    powerup_state.current_player = PLAYER_ONE;
    terminal_init_board(&powerup_state);
    powerup_state.move_count = 1;
    powerup_state.piece_count = 1;
    powerup_state.board[0][6] = P1;
    powerup_state.board[1][6] = P2;
    powerup_state.board[2][6] = P1;
    powerup_state.move_count = 3;
    powerup_state.piece_count = 3;
    assert(rules_validate_state(&powerup_state));
    assert(rules_is_valid_move(&powerup_state, anvil_move));

    GameState full_powerup = powerup_state;
    full_powerup.rows = 4;
    full_powerup.cols = 4;
    full_powerup.win_length = 4;
    for (int r = 0; r < full_powerup.rows; r++) {
        for (int c = 0; c < full_powerup.cols; c++) {
            full_powerup.board[r][c] = draw_board[r][c];
        }
    }
    full_powerup.move_count = full_powerup.rows * full_powerup.cols;
    full_powerup.piece_count = full_powerup.move_count;
    full_powerup.current_player = PLAYER_ONE;
    full_powerup.bomb_used[PLAYER_ONE] = false;
    full_powerup.anvil_used[PLAYER_ONE] = false;
    assert(rules_validate_state(&full_powerup));
    GameState impossible_count = full_powerup;
    impossible_count.move_count++;
    impossible_count.move_count++;
    impossible_count.move_count++;
    impossible_count.move_count++;
    impossible_count.move_count++;
    assert(!rules_validate_state(&impossible_count));
    assert(!rules_is_valid_move(&full_powerup, (Move){0, MOVE_NORMAL}));
    assert(!rules_is_valid_move(&full_powerup, (Move){0, MOVE_BOMB}));
    assert(rules_is_valid_move(&full_powerup, (Move){0, MOVE_ANVIL}));
    GameState after_anvil = {0};
    assert(rules_apply_move(&full_powerup,
                            (Move){0, MOVE_ANVIL}, &after_anvil));
    assert(after_anvil.piece_count == full_powerup.rows * (full_powerup.cols - 1));
    assert(!rules_is_draw(&full_powerup));

    full_powerup = after_anvil;
    for (int r = 0; r < full_powerup.rows; r++) {
        full_powerup.board[r][0] = draw_board[r][0];
    }
    full_powerup.piece_count = full_powerup.rows * full_powerup.cols;
    full_powerup.anvil_used[PLAYER_TWO] = true;
    full_powerup.bomb_used[PLAYER_ONE] = true;
    full_powerup.bomb_used[PLAYER_TWO] = true;
    assert(rules_is_draw(&full_powerup));

    assert(rules_is_valid_move(&game, (Move){0, MOVE_NORMAL}));
    assert(!rules_is_valid_move(&game, (Move){-1, MOVE_NORMAL}));

    GameState drawn = {0};
    drawn.rows = 4;
    drawn.cols = 4;
    drawn.win_length = 4;
    drawn.move_count = 16;
    drawn.piece_count = 16;
    drawn.current_player = PLAYER_ONE;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            drawn.board[r][c] = draw_board[r][c];
        }
    }
    assert(rules_check_winner(&drawn) == 0);
    assert(rules_is_draw(&drawn));
    /* With power-ups on, a full board isn't a draw while anvils are left. */
    drawn.powerups = true;
    assert(!rules_is_draw(&drawn));

    GameState horizontal = {0};
    horizontal.rows = 6;
    horizontal.cols = 7;
    horizontal.win_length = 4;
    horizontal.move_count = 4;
    horizontal.piece_count = 4;
    horizontal.current_player = PLAYER_ONE;
    terminal_init_board(&horizontal);
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


    GameState before_bomb = horizontal;
    terminal_init_board(&before_bomb);
    before_bomb.powerups = true;
    before_bomb.board[0][0] = P1;
    before_bomb.board[0][1] = P2;
    before_bomb.board[0][2] = P1;
    before_bomb.move_count = 3;
    before_bomb.piece_count = 3;
    before_bomb.current_player = PLAYER_ONE;

    GameState after_bomb = {0};
    assert(rules_apply_move(&before_bomb,
                            (Move){1, MOVE_BOMB}, &after_bomb));
    assert(after_bomb.piece_count == 0);
    assert(after_bomb.move_count == 4);
    assert(after_bomb.bomb_used[PLAYER_ONE] == true);
    assert(rules_validate_state(&after_bomb));

    return 0;
}
