#ifndef STATS_H
#define STATS_H

/*
 * Scores, win streaks and the leaderboard.
 *
 * Leaderboard file layout:
 *   "C4LB"                      4 bytes  file ID
 *   version                     1 byte
 *   player count                4 bytes
 *   one record per player      40 bytes each:
 *     name                     16 bytes, zero padded
 *     score, wins, losses, draws,
 *     current streak, best streak   4 bytes each
 *   CRC-32                      4 bytes, checksum of everything above
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "header.h"

#define LEADERBOARD_FILE "connect4_leaderboard.bin"
#define LEADERBOARD_SHOWN 10

#define RESULT_LOSS (-1)
#define RESULT_DRAW 0
#define RESULT_WIN 1

#define DRAW_POINTS 20
#define UNDO_PENALTY 10
#define MIN_WIN_POINTS 10

typedef struct {
    char name[NAME_LEN];
    int32_t score;
    int32_t wins;
    int32_t losses;
    int32_t draws;
    int32_t current_streak;
    int32_t best_streak;
} PlayerStats;

/* Names match without caring about capitals, so "Ali" and "ali" are one player. */
bool stats_same_name(const char *a, const char *b);

/* Points for winning this finished game:
 *   (100 + board bonus + speed bonus) x difficulty - 10 per undo, at least 10
 *   board bonus = rows * cols - 42        (0 on the classic board)
 *   speed bonus = rows * cols - moves     (0 if there were more moves)
 * multiplier is 1 for Human vs Human, else the AI level (1, 2 or 3).
 * If explain is not NULL, the sum is written into it for showing the player. */
int stats_win_points(const GameState *game, int multiplier, int undos_used,
                     char *explain, size_t size);

/* Loads every player into a malloc'd array (free it with free()).
 * Returns 1 on success, 0 if there is no file yet (empty list), -1 if the
 * file is damaged. */
int stats_load(const char *path, PlayerStats **players, int *count);

/* Writes the whole list. Returns 1 on success, 0 on failure. */
int stats_save(const char *path, const PlayerStats *players, int count);

/* Adds one finished game to a player's record, adding them if they are new.
 * result is RESULT_WIN, RESULT_DRAW or RESULT_LOSS. The updated record is
 * copied into out if it is not NULL. Returns 1 on success, 0 on failure. */
int stats_record(const char *path, const char *name, int result, int points,
                 PlayerStats *out);

/* Prints the top players by score. */
void stats_show_leaderboard(const char *path);

#endif
