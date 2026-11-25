#include "rules.h"
#include <limits.h>
#include <stddef.h>

bool rules_validate_state(const GameState *state) {
    if (!state || state->rows < MIN_SIZE || state->rows > MAX_SIZE ||
        state->cols < MIN_SIZE || state->cols > MAX_SIZE ||
        (state->win_length != 4 && state->win_length != 5) ||
        (state->win_length > state->rows && state->win_length > state->cols) ||
        (state->current_player != PLAYER_ONE && state->current_player != PLAYER_TWO) ||
        state->move_count < 0 ||
        (!state->powerups && state->move_count > state->rows * state->cols) ||
        state->piece_count < 0 || state->piece_count > state->move_count ||
        state->piece_count > state->rows * state->cols) {
        return false;
    }

    int piece_count = 0;
    int p1_count = 0;
    int p2_count = 0;

    for (int c = 0; c < state->cols; c++) {
        bool seen_empty = false;
        for (int r = 0; r < state->rows; r++) {
            uint8_t cell = state->board[r][c];
            if (cell > P2) return false;
            if (cell == EMPTY) {
                seen_empty = true;
            } else {
                if (seen_empty) return false;
                piece_count++;
                if (cell == P1) p1_count++;
                else p2_count++;
            }
        }
    }

    if (piece_count != state->piece_count) {
        return false;
    }

    if (state->powerups) {
        /* Each move adds at most one piece, and each bomb (up to 8 pieces) or
         * anvil (up to a full column) removes some. So the move count can't be
         * more than the pieces left + everything power-ups could have removed
         * + the power-up moves themselves (which add no piece). */
        int powerups_used = 0;
        int max_removed = 0;
        for (int p = PLAYER_ONE; p <= PLAYER_TWO; p++) {
            if (state->bomb_used[p]) {
                powerups_used++;
                max_removed += 8;
            }
            if (state->anvil_used[p]) {
                powerups_used++;
                max_removed += state->rows;
            }
        }
        if (state->move_count > state->piece_count + max_removed + powerups_used) {
            return false;
        }
        return true;
    }

    /* Classic rules: no power-up can have been used, every move added
     * exactly one piece, and the players took turns. */
    for (int p = PLAYER_ONE; p <= PLAYER_TWO; p++) {
        if (state->bomb_used[p] || state->anvil_used[p]) return false;
    }
    if (state->move_count != state->piece_count) {
        return false;
    }
    if (p1_count < p2_count || p1_count - p2_count > 1) return false;
    if ((p1_count == p2_count && state->current_player != PLAYER_ONE) ||
        (p1_count == p2_count + 1 &&
         state->current_player != PLAYER_TWO)) {
        return false;
    }
    return true;
}

bool rules_is_valid_move(const GameState *state, Move move) {
    if (!rules_validate_state(state) ||
        move.col < 0 || move.col >= state->cols ||
        rules_check_winner(state) != 0) {
        return false;
    }
    bool col_open = state->board[state->rows - 1][move.col] == EMPTY;
    int p = state->current_player;

    if (move.type == MOVE_NORMAL) return col_open;
    if (!state->powerups) return false;   /* bomb/anvil need power-ups on */
    if (move.type == MOVE_BOMB) return col_open && !state->bomb_used[p];
    if (move.type == MOVE_ANVIL) return !state->anvil_used[p];
    return false;
}

bool rules_apply_move(const GameState *state, Move move, GameState *out) {
    if (!state || !out || !rules_validate_state(state)) {
        return false;
    }
    if (!rules_is_valid_move(state, move)) {
        *out = *state;
        return false;
    }

    /* rules_is_valid_move has already checked power-ups are on and unused. */
    GameState next = *state;
    int p = next.current_player;

    if (move.type == MOVE_NORMAL) {
        for (int r = 0; r < next.rows; r++) {
            if (next.board[r][move.col] == EMPTY) {
                next.board[r][move.col] = (uint8_t)p;
                break;
            }
        }
    } else if (move.type == MOVE_BOMB) {
        /* Lands on the first empty row, then clears the 3x3 around it. */
        int land_row = next.rows - 1;
        for (int r = 0; r < next.rows; r++) {
            if (next.board[r][move.col] == EMPTY) {
                land_row = r;
                break;
            }
        }
        int r_min = land_row > 0 ? land_row - 1 : 0;
        int r_max = land_row + 1 < next.rows ? land_row + 1 : next.rows - 1;
        int c_min = move.col > 0 ? move.col - 1 : 0;
        int c_max = move.col + 1 < next.cols ? move.col + 1 : next.cols - 1;
        for (int r = r_min; r <= r_max; r++) {
            for (int c = c_min; c <= c_max; c++) {
                next.board[r][c] = EMPTY;
            }
        }
        next.bomb_used[p] = true;
        rules_apply_gravity(&next);
    } else {
        /* Anvil: clears the whole column. */
        for (int r = 0; r < next.rows; r++) {
            next.board[r][move.col] = EMPTY;
        }
        next.anvil_used[p] = true;
    }

    if (next.move_count >= INT_MAX) {
        *out = *state;
        return false;
    }
    next.move_count++;
    int piece_count = 0;
    for (int r = 0; r < next.rows; r++) {
        for (int c = 0; c < next.cols; c++) {
            if (next.board[r][c] != EMPTY) piece_count++;
        }
    }
    next.piece_count = piece_count;
    next.current_player = (p == PLAYER_ONE) ? PLAYER_TWO : PLAYER_ONE;
    *out = next;
    return true;
}

void rules_apply_gravity(GameState *state) {
    if (!state || state->rows < MIN_SIZE || state->rows > MAX_SIZE ||
        state->cols < MIN_SIZE || state->cols > MAX_SIZE) {
        return;
    }
    for (int c = 0; c < state->cols; c++) {
        int write_row = 0;
        for (int r = 0; r < state->rows; r++) {
            uint8_t piece = state->board[r][c];
            if (piece != EMPTY) {
                state->board[r][c] = EMPTY;
                state->board[write_row][c] = piece;
                write_row++;
            }
        }
        for (int r = write_row; r < state->rows; r++) {
            state->board[r][c] = EMPTY;
        }
    }
}

int rules_check_winner(const GameState *state) {
    static const int directions[4][2] = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1}
    };
    if (!rules_validate_state(state)) return 0;

    bool p1_won = false;
    bool p2_won = false;
    for (int r = 0; r < state->rows; r++) {
        for (int c = 0; c < state->cols; c++) {
            uint8_t piece = state->board[r][c];
            if (piece == EMPTY) continue;

            for (int d = 0; d < 4; d++) {
                int end_r = r + (state->win_length - 1) * directions[d][0];
                int end_c = c + (state->win_length - 1) * directions[d][1];
                if (end_r < 0 || end_r >= state->rows ||
                    end_c < 0 || end_c >= state->cols) {
                    continue;
                }

                bool matched = true;
                for (int k = 1; k < state->win_length; k++) {
                    if (state->board[r + k * directions[d][0]]
                                     [c + k * directions[d][1]] != piece) {
                        matched = false;
                        break;
                    }
                }
                if (matched) {
                    if (piece == P1) p1_won = true;
                    if (piece == P2) p2_won = true;
                }
            }
        }
    }

    if (p1_won && p2_won) return 3;
    if (p1_won) return P1;
    if (p2_won) return P2;
    return 0;
}

bool rules_is_draw(const GameState *state) {
    if (!rules_validate_state(state)) {
        return false;
    }
    int winner = rules_check_winner(state);
    if (winner == 3) return true;
    if (winner != 0) return false;

    if (state->powerups) {
        /* A full board isn't a draw while someone can still use an anvil:
         * it's a draw only when no move of any type is possible. */
        for (int c = 0; c < state->cols; c++) {
            Move moves[] = {
                {c, MOVE_NORMAL},
                {c, MOVE_BOMB},
                {c, MOVE_ANVIL}
            };
            for (size_t i = 0; i < sizeof moves / sizeof moves[0]; i++) {
                if (rules_is_valid_move(state, moves[i])) return false;
            }
        }
        return true;
    }

    for (int r = 0; r < state->rows; r++) {
        for (int c = 0; c < state->cols; c++) {
            if (state->board[r][c] == EMPTY) return false;
        }
    }
    return true;
}

bool rules_is_terminal(const GameState *state) {
    return rules_check_winner(state) != 0 || rules_is_draw(state);
}
