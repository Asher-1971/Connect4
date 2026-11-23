#ifndef MODULE3_H
#define MODULE3_H

#include <stdbool.h>
#include <stdint.h>

#include "header.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Board & Compile-Time Feature Configuration */
#define MAX_DIM 20
#define ENABLE_POWERUPS 0  /* Flip to 1 in v2 to enable Bomb & Anvil moves */

/* Stack-allocated search telemetry & re-entrant timeout budget */
typedef struct {
    uint64_t nodes;
    uint64_t deadline_ms;
    bool timed_out;
} SearchContext;

/* Core AI Engine API */
int generate_moves(const GameState *s, Move moves[MAX_DIM * 3]);
void apply_gravity(GameState *s);
bool try_apply_move(const GameState *s, Move m, GameState *out);
GameState apply_move(GameState s, Move m);
int check_winner(const GameState *s);
int evaluate_board(const GameState *s);
int negamax(const GameState *s, int depth, int alpha, int beta, SearchContext *ctx);
Move find_best_move(const GameState *root, int time_budget_ms);

/* Stable AI bridge for the canonical numeric bottom-up GameState. */
static inline int getBestMove(const GameState *fe, int win_length, int time_budget_ms) {
    if (!fe || fe->rows <= 0 || fe->rows > MAX_DIM ||
        fe->cols <= 0 || fe->cols > MAX_DIM ||
        (win_length != 4 && win_length != 5) ||
        fe->win_length != win_length ||
        (win_length > fe->rows && win_length > fe->cols) ||
        (fe->current_player != PLAYER_ONE && fe->current_player != PLAYER_TWO) ||
        fe->move_count < 0 || fe->move_count > fe->rows * fe->cols ||
        fe->piece_count < 0 || fe->piece_count > fe->move_count ||
        fe->piece_count > fe->rows * fe->cols) {
        return -1;
    }

    Move best = find_best_move(fe, time_budget_ms);
    if (best.col < 0 || best.col >= fe->cols) {
        return -1;
    }
    return best.col + 1;
}

#ifdef __cplusplus
}
#endif

#endif /* MODULE3_H */
