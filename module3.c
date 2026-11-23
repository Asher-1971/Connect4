#ifndef _WIN32
#  define _POSIX_C_SOURCE 200809L
#endif

#include "module3.h"
#include "rules.h"
#include <stdlib.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <time.h>
#endif

#define WIN_SCORE 1000000
#define INF_SCORE 20000000
#define MAX_SEARCH_DEPTH 64

/* Cross-platform monotonic timestamp in milliseconds. */
static bool now_ms(uint64_t *result) {
    if (!result) {
        return false;
    }
#ifdef _WIN32
    *result = (uint64_t)GetTickCount64();
    return true;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return false;
    }
    *result = (uint64_t)ts.tv_sec * 1000ULL +
              (uint64_t)ts.tv_nsec / 1000000ULL;
    return true;
#endif
}

/* Builds center-out column ordering: e.g., for cols=7 -> [3, 2, 4, 1, 5, 0, 6] */
static void build_center_out_order(int cols, int order[MAX_DIM]) {
    int mid = cols / 2;
    int idx = 0;
    order[idx++] = mid;

    for (int offset = 1; idx < cols; offset++) {
        int left = mid - offset;
        int right = mid + offset;
        if (left >= 0) {
            order[idx++] = left;
        }
        if (right < cols && idx < cols) {
            order[idx++] = right;
        }
    }
}

/*
 * Generates legal moves ordered center-outward.
 * Guards MOVE_NORMAL (and v2 MOVE_BOMB) with !col_full without skipping
 * full-column checks for v2 MOVE_ANVIL.
 */
int generate_moves(const GameState *s, Move moves[MAX_DIM * 3]) {
    if (!rules_validate_state(s) || !moves || rules_is_terminal(s)) {
        return 0;
    }

    int col_order[MAX_DIM];
    build_center_out_order(s->cols, col_order);

    int count = 0;
#if ENABLE_POWERUPS
    int p = s->current_player;
    if (p != 1 && p != 2) {
        return 0;
    }
#endif

    for (int i = 0; i < s->cols; i++) {
        int col = col_order[i];
        Move normal_move = {col, MOVE_NORMAL};
        bool col_full = !rules_is_valid_move(s, normal_move);

        if (!col_full) {
            moves[count].col = col;
            moves[count].type = MOVE_NORMAL;
            count++;
        }

#if ENABLE_POWERUPS
        if (!col_full && !s->bomb_used[p]) {
            moves[count].col = col;
            moves[count].type = MOVE_BOMB;
            count++;
        }
        if (!s->anvil_used[p]) {
            moves[count].col = col;
            moves[count].type = MOVE_ANVIL;
            count++;
        }
#endif
    }

    return count;
}

/*
 * Scans the entire board for both players to detect simultaneous double-wins.
 * Returns: 0 = ongoing, 1 = Player 1 wins, 2 = Player 2 wins, 3 = simultaneous draw.
 */
int check_winner(const GameState *s) {
    return rules_check_winner(s);
}

/* Scores a single K-cell slice from 'me' perspective */
static inline int score_window(int my_cnt, int opp_cnt, int K) {
    if (my_cnt > 0 && opp_cnt > 0) {
        return 0; /* Contested window cannot produce a win */
    }
    if (my_cnt == 0 && opp_cnt == 0) {
        return 0;
    }

    if (opp_cnt == 0) {
        if (my_cnt == K) {
            return 100000;
        }
        if (my_cnt == K - 1) {
            return (K == 5) ? 650 : 500;
        }
        if (my_cnt == K - 2) {
            return (K == 5) ? 90 : 45;
        }
        if (K == 5 && my_cnt == K - 3) {
            return 15;
        }
    } else {
        if (opp_cnt == K) {
            return -100000;
        }
        if (opp_cnt == K - 1) {
            return (K == 5) ? -900 : -700; /* Heavy threat penalty forces blocking */
        }
        if (opp_cnt == K - 2) {
            return (K == 5) ? -120 : -60;  /* Penalize unblocked 3-in-a-row in Connect-5 */
        }
        if (K == 5 && opp_cnt == K - 3) {
            return -20;
        }
    }
    return 0;
}

/*
 * Generalized K-window static evaluator strictly from s->current_player's view.
 * Anti-symmetric across players so Negamax sign flips remain exact.
 */
int evaluate_board(const GameState *s) {
    static const int dirs[4][2] = {
        {0,  1},
        {1,  0},
        {1,  1},
        {1, -1}
    };

    if (!rules_validate_state(s)) {
        return 0;
    }

    int me = s->current_player;
    int opp = (me == 1) ? 2 : 1;
    int K = s->win_length;
    int score = 0;

    /* 1. Center-column proximity bonus scaled dynamically to board width */
    int max_dist2 = (s->cols > 1) ? (s->cols - 1) : 1;
    for (int c = 0; c < s->cols; c++) {
        int dist2 = abs(2 * c - (s->cols - 1));
        int col_weight = 1 + (6 * (max_dist2 - dist2)) / max_dist2;
        for (int r = 0; r < s->rows; r++) {
            uint8_t cell = s->board[r][c];
            if (cell == (uint8_t)me) {
                score += col_weight;
            } else if (cell == (uint8_t)opp) {
                score -= col_weight;
            }
        }
    }

    /* 2. Slide K-length windows across all 4 directions */
    for (int r = 0; r < s->rows; r++) {
        for (int c = 0; c < s->cols; c++) {
            for (int d = 0; d < 4; d++) {
                int end_r = r + (K - 1) * dirs[d][0];
                int end_c = c + (K - 1) * dirs[d][1];

                if (end_r < 0 || end_r >= s->rows || end_c < 0 || end_c >= s->cols) {
                    continue;
                }

                int my_cnt = 0;
                int opp_cnt = 0;
                for (int step = 0; step < K; step++) {
                    uint8_t cell = s->board[r + step * dirs[d][0]][c + step * dirs[d][1]];
                    if (cell == (uint8_t)me) {
                        my_cnt++;
                    } else if (cell == (uint8_t)opp) {
                        opp_cnt++;
                    }
                }
                score += score_window(my_cnt, opp_cnt, K);
            }
        }
    }

#if ENABLE_POWERUPS
    /* 3. Symmetric power-up conservation penalty */
    if (s->bomb_used[me])   score -= 25;
    if (s->anvil_used[me])  score -= 25;
    if (s->bomb_used[opp])  score += 25;
    if (s->anvil_used[opp]) score += 25;
#endif

    return score;
}

/* Depth-limited Negamax with Alpha-Beta pruning and periodic timer checks */
int negamax(const GameState *s, int depth, int alpha, int beta, SearchContext *ctx) {
    if (!rules_validate_state(s) || !ctx) {
        return 0;
    }
    if ((ctx->nodes++ & 1023ULL) == 0ULL) {
        uint64_t current_ms;
        if (!now_ms(&current_ms) || current_ms >= ctx->deadline_ms) {
            ctx->timed_out = true;
            return 0;
        }
    }
    if (ctx->timed_out) {
        return 0;
    }

    int winner = check_winner(s);
    if (winner == 3) {
        return 0; /* Simultaneous double-win is a draw */
    }
    if (winner != 0) {
        return (winner == s->current_player) ? (WIN_SCORE + depth) : (-WIN_SCORE - depth);
    }

    /* Evaluate leaf nodes before generating moves to avoid wasted work */
    if (depth <= 0) {
        return evaluate_board(s);
    }

    Move moves[MAX_DIM * 3];
    int num_moves = generate_moves(s, moves);
    if (num_moves == 0) {
        return 0; /* Full board draw */
    }

    int best_val = -INF_SCORE;
    for (int i = 0; i < num_moves; i++) {
        GameState next;
        if (!rules_apply_move(s, moves[i], &next)) {
            continue;
        }
        int val = -negamax(&next, depth - 1, -beta, -alpha, ctx);

        if (ctx->timed_out) {
            return 0;
        }

        if (val > best_val) {
            best_val = val;
        }
        if (val > alpha) {
            alpha = val;
        }
        if (alpha >= beta) {
            break; /* Beta cutoff */
        }
    }

    return best_val;
}

/* Promotes the Principal Variation move from the previous completed depth to index 0 */
static void promote_pv_move(Move moves[MAX_DIM * 3], int num_moves, Move pv) {
    for (int i = 1; i < num_moves; i++) {
        if (moves[i].col == pv.col && moves[i].type == pv.type) {
            Move tmp = moves[i];
            for (int j = i; j > 0; j--) {
                moves[j] = moves[j - 1];
            }
            moves[0] = tmp;
            return;
        }
    }
}

/*
 * Iterative Deepening Negamax root driver with wall-clock time budget.
 * Discards incomplete depths on timeout and returns the last fully completed PV move.
 */
Move find_best_move(const GameState *root, int time_budget_ms) {
    Move invalid = { -1, MOVE_NORMAL };
    if (!rules_validate_state(root) || rules_is_terminal(root)) {
        return invalid;
    }

    Move root_moves[MAX_DIM * 3];
    int num_moves = generate_moves(root, root_moves);
    if (num_moves == 0) {
        return invalid;
    }

    Move best_move = root_moves[0];
    if (num_moves == 1 || time_budget_ms <= 0) {
        return best_move;
    }


    SearchContext ctx;
    ctx.nodes = 0ULL;
    uint64_t start_ms;
    if (!now_ms(&start_ms)) {
        return best_move;
    }
    ctx.deadline_ms = start_ms + (uint64_t)time_budget_ms;
    ctx.timed_out = false;

    int max_depth = MAX_SEARCH_DEPTH;
#if !ENABLE_POWERUPS
    int remaining = (root->rows * root->cols) - root->move_count;
    max_depth = (remaining > 0) ? remaining : 1;
    if (max_depth > MAX_SEARCH_DEPTH) {
        max_depth = MAX_SEARCH_DEPTH;
    }
#endif

    for (int depth = 1; depth <= max_depth; depth++) {
        promote_pv_move(root_moves, num_moves, best_move);

        int alpha = -INF_SCORE;
        int beta = INF_SCORE;
        int iter_best_score = -INF_SCORE;
        Move iter_best_move = root_moves[0];

        for (int i = 0; i < num_moves; i++) {
            GameState next;
            if (!rules_apply_move(root, root_moves[i], &next)) {
                continue;
            }
            int score = -negamax(&next, depth - 1, -beta, -alpha, &ctx);

            if (ctx.timed_out) {
                break;
            }

            if (score > iter_best_score) {
                iter_best_score = score;
                iter_best_move = root_moves[i];
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        if (ctx.timed_out) {
            break; /* Discard partial depth; keep best_move from last completed depth */
        }

        best_move = iter_best_move;

        /* Stop early once a forced win or forced loss is proven at this depth */
        if (abs(iter_best_score) >= WIN_SCORE) {
            break;
        }
    }

    return best_move;
}
