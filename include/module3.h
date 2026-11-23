#ifndef MODULE3_H
#define MODULE3_H

#include <stdbool.h>
#include <stdint.h>

#include "header.h"
#include "rules.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Board & Compile-Time Feature Configuration */
#define MAX_DIM 20

/* Stack-allocated search telemetry & re-entrant timeout budget */
typedef struct {
    uint64_t nodes;
    uint64_t deadline_ms;
    bool timed_out;
} SearchContext;

/* Core AI Engine API */
int generate_moves(const GameState *s, Move moves[MAX_DIM * 3]);
int check_winner(const GameState *s);
int evaluate_board(const GameState *s);
int negamax(const GameState *s, int depth, int alpha, int beta, SearchContext *ctx);
Move find_best_move(const GameState *root, int time_budget_ms);

/* Stable AI bridge for the canonical numeric bottom-up GameState. */
static inline Move getBestMoveMove(const GameState *fe, int win_length, int time_budget_ms) {
    Move invalid = {-1, MOVE_NORMAL};
    if (!rules_validate_state(fe) || fe->win_length != win_length) {
        return invalid;
    }

    Move best = find_best_move(fe, time_budget_ms);
    if (best.col < 0 || best.col >= fe->cols) return invalid;
    return best;
}

static inline int getBestMove(const GameState *fe, int win_length, int time_budget_ms) {
    Move best = getBestMoveMove(fe, win_length, time_budget_ms);
    return best.col < 0 ? -1 : best.col + 1;
}

#ifdef __cplusplus
}
#endif

#endif /* MODULE3_H */
