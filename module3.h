#ifndef MODULE3_H
#define MODULE3_H

#include <stdbool.h>
#include <stdint.h>

/* Auto-include frontend header if present so include order never matters */
#if defined(__has_include)
#  if __has_include("header.h")
#    include "header.h"
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Board & Compile-Time Feature Configuration */
#define MAX_DIM 20
#define ENABLE_POWERUPS 0  /* Flip to 1 in v2 to enable Bomb & Anvil moves */

/* Move abstractions are shared with the canonical frontend GameState. */
#ifndef HEADER_H
typedef enum {
    MOVE_NORMAL = 0,
    MOVE_BOMB = 1,
    MOVE_ANVIL = 2
} MoveType;

typedef struct {
    int col;
    MoveType type;
} Move;
#endif

/*
 * Flat, pointer-free Copy-Make state (~428 bytes).
 * Board convention: row 0 = bottom, row (rows - 1) = top.
 * Cell values: 0 = empty, 1 = Player 1, 2 = Player 2.
 */
typedef struct AIState {
    uint8_t board[MAX_DIM][MAX_DIM];
    int rows;
    int cols;
    int win_length;        /* 4 for Connect-4, 5 for Connect-5 */
    int current_player;    /* 1 or 2 */
    int move_count;
    bool bomb_used[3];     /* Indexed by player (1 or 2) */
    bool anvil_used[3];    /* Indexed by player (1 or 2) */
} AIGameState;

/* Allow standalone tests to refer to AIGameState as GameState when header.h is absent */
#ifndef HEADER_H
typedef AIGameState GameState;
#endif

/* Stack-allocated search telemetry & re-entrant timeout budget */
typedef struct {
    uint64_t nodes;
    uint64_t deadline_ms;
    bool timed_out;
} SearchContext;

/* Core AI Engine API */
int generate_moves(const AIGameState *s, Move moves[MAX_DIM * 3]);
void apply_gravity(AIGameState *s);
bool try_apply_move(const AIGameState *s, Move m, AIGameState *out);
AIGameState apply_move(AIGameState s, Move m);
int check_winner(const AIGameState *s);
int evaluate_board(const AIGameState *s);
int negamax(const AIGameState *s, int depth, int alpha, int beta, SearchContext *ctx);
Move find_best_move(const AIGameState *root, int time_budget_ms);

/*
 * Seamless Bridge for Frontend (header.h / start.c):
 * Converts top-down char board ('.', 'X', 'O') to bottom-up AIGameState.
 * Returns a 1-indexed column (1..cols), or -1 if no legal move exists.
 */
#ifdef HEADER_H
static inline int getBestMove(const GameState *fe, int win_length, int time_budget_ms) {
    if (!fe || fe->rows <= 0 || fe->rows > MAX_DIM ||
        fe->cols <= 0 || fe->cols > MAX_DIM ||
        win_length < 4 || win_length > MAX_DIM ||
        (win_length > fe->rows && win_length > fe->cols) ||
        (fe->current_player != PLAYER_ONE && fe->current_player != PLAYER_TWO) ||
        fe->move_count < 0 || fe->move_count > fe->rows * fe->cols ||
        fe->piece_count < 0 || fe->piece_count > fe->rows * fe->cols) {
        return -1;
    }

    AIGameState ai = {0};
    ai.rows = fe->rows;
    ai.cols = fe->cols;
    ai.win_length = win_length;
    ai.current_player = fe->current_player;
    ai.move_count = fe->move_count;

    int pieces = 0;
    for (int r = 0; r < fe->rows; r++) {
        for (int c = 0; c < fe->cols; c++) {
            uint8_t cell = fe->board[r][c];
            if (cell == PLAYER_ONE) {
                ai.board[r][c] = PLAYER_ONE;
                pieces++;
            } else if (cell == PLAYER_TWO) {
                ai.board[r][c] = PLAYER_TWO;
                pieces++;
            } else if (cell == EMPTY) {
                ai.board[r][c] = PLAYER_NONE;
            } else {
                return -1;
            }
        }
    }
    if (pieces != fe->piece_count || pieces != fe->move_count) {
        return -1;
    }

    Move best = find_best_move(&ai, time_budget_ms);
    if (best.col < 0 || best.col >= fe->cols) {
        return -1;
    }
    return best.col + 1;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* MODULE3_H */
