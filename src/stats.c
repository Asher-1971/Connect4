#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stats.h"
#include "saveload.h"   /* save_crc32 */

#define STATS_MAGIC   "C4LB"
#define STATS_VERSION 1
#define STATS_HEADER  9                 /* magic + version + count */
#define RECORD_SIZE   (NAME_LEN + 6 * 4)
#define MAX_PLAYERS_SAVED 100000        /* sanity limit when loading */

bool stats_same_name(const char *a, const char *b)
{
    while (*a && tolower((unsigned char)*a) == tolower((unsigned char)*b)) {
        a++;
        b++;
    }
    return tolower((unsigned char)*a) == tolower((unsigned char)*b);
}

int stats_win_points(const GameState *game, int multiplier, int undos_used,
                     char *explain, size_t size)
{
    int cells = game->rows * game->cols;
    int board_bonus = cells - 42;
    int speed_bonus = cells - game->move_count;
    if (speed_bonus < 0) speed_bonus = 0;
    if (multiplier < 1) multiplier = 1;

    int points = (100 + board_bonus + speed_bonus) * multiplier
               - UNDO_PENALTY * undos_used;
    if (points < MIN_WIN_POINTS) points = MIN_WIN_POINTS;

    if (explain) {
        snprintf(explain, size,
                 "(100 + %d board + %d speed) x %d - %d for undos = %d",
                 board_bonus, speed_bonus, multiplier,
                 UNDO_PENALTY * undos_used, points);
    }
    return points;
}

/* Little helpers so the file is the same on every computer. */
static void put32(uint8_t *p, int32_t v) { memcpy(p, &v, 4); }
static int32_t get32(const uint8_t *p) { int32_t v; memcpy(&v, p, 4); return v; }

int stats_load(const char *path, PlayerStats **players, int *count)
{
    *players = NULL;
    *count = 0;

    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;

    /* Read the whole file into memory, growing the buffer as needed. */
    size_t cap = 1024, len = 0;
    uint8_t *buf = malloc(cap);
    if (!buf) {
        fclose(fp);
        return -1;
    }
    size_t got;
    while ((got = fread(buf + len, 1, cap - len, fp)) > 0) {
        len += got;
        if (len == cap) {
            uint8_t *bigger = realloc(buf, cap * 2);
            if (!bigger) {
                free(buf);
                fclose(fp);
                return -1;
            }
            buf = bigger;
            cap *= 2;
        }
    }
    fclose(fp);

    /* Check ID, version, size and CRC before trusting anything. */
    int32_t n = len >= STATS_HEADER ? get32(buf + 5) : -1;
    if (len < STATS_HEADER + 4 || memcmp(buf, STATS_MAGIC, 4) != 0 ||
        buf[4] != STATS_VERSION || n < 0 || n > MAX_PLAYERS_SAVED ||
        len != (size_t)STATS_HEADER + (size_t)n * RECORD_SIZE + 4) {
        free(buf);
        return -1;
    }
    uint32_t stored_crc;
    memcpy(&stored_crc, buf + len - 4, 4);
    if (save_crc32(buf, len - 4) != stored_crc) {
        free(buf);
        return -1;
    }

    PlayerStats *list = malloc((size_t)(n > 0 ? n : 1) * sizeof *list);
    if (!list) {
        free(buf);
        return -1;
    }
    const uint8_t *p = buf + STATS_HEADER;
    for (int i = 0; i < n; i++, p += RECORD_SIZE) {
        memcpy(list[i].name, p, NAME_LEN);
        list[i].name[NAME_LEN - 1] = '\0';
        list[i].score = get32(p + NAME_LEN);
        list[i].wins = get32(p + NAME_LEN + 4);
        list[i].losses = get32(p + NAME_LEN + 8);
        list[i].draws = get32(p + NAME_LEN + 12);
        list[i].current_streak = get32(p + NAME_LEN + 16);
        list[i].best_streak = get32(p + NAME_LEN + 20);
    }
    free(buf);

    *players = list;
    *count = n;
    return 1;
}

int stats_save(const char *path, const PlayerStats *players, int count)
{
    if (!path || count < 0 || (count > 0 && !players)) return 0;

    size_t len = STATS_HEADER + (size_t)count * RECORD_SIZE + 4;
    uint8_t *buf = calloc(len, 1);
    if (!buf) return 0;

    memcpy(buf, STATS_MAGIC, 4);
    buf[4] = STATS_VERSION;
    put32(buf + 5, count);
    uint8_t *p = buf + STATS_HEADER;
    for (int i = 0; i < count; i++, p += RECORD_SIZE) {
        memcpy(p, players[i].name, strlen(players[i].name));
        put32(p + NAME_LEN, players[i].score);
        put32(p + NAME_LEN + 4, players[i].wins);
        put32(p + NAME_LEN + 8, players[i].losses);
        put32(p + NAME_LEN + 12, players[i].draws);
        put32(p + NAME_LEN + 16, players[i].current_streak);
        put32(p + NAME_LEN + 20, players[i].best_streak);
    }
    uint32_t crc = save_crc32(buf, len - 4);
    memcpy(buf + len - 4, &crc, 4);

    /* Same safe write as the save files: temp file, then swap it in. */
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *fp = fopen(tmp, "wb");
    if (!fp) {
        free(buf);
        return 0;
    }
    size_t written = fwrite(buf, 1, len, fp);
    free(buf);
    if (fclose(fp) != 0 || written != len) {
        remove(tmp);
        return 0;
    }
    remove(path);                 /* Windows rename() won't overwrite */
    return rename(tmp, path) == 0;
}

int stats_record(const char *path, const char *name, int result, int points,
                 PlayerStats *out)
{
    PlayerStats *players;
    int count;
    if (stats_load(path, &players, &count) == -1) {
        printf("The leaderboard file was damaged, so a new one was started.\n");
    }

    int i = 0;
    while (i < count && !stats_same_name(players[i].name, name)) i++;

    if (i == count) {   /* new player: grow the list by one */
        PlayerStats *bigger = realloc(players, (size_t)(count + 1) * sizeof *bigger);
        if (!bigger) {
            free(players);
            return 0;
        }
        players = bigger;
        memset(&players[i], 0, sizeof players[i]);
        snprintf(players[i].name, NAME_LEN, "%s", name);
        count++;
    }

    PlayerStats *ps = &players[i];
    ps->score += points;
    if (result == RESULT_WIN) {
        ps->wins++;
        ps->current_streak++;
        if (ps->current_streak > ps->best_streak) {
            ps->best_streak = ps->current_streak;
        }
    } else if (result == RESULT_LOSS) {
        ps->losses++;
        ps->current_streak = 0;
    } else {
        ps->draws++;    /* a draw keeps the streak as it is */
    }
    if (out) *out = *ps;

    int ok = stats_save(path, players, count);
    free(players);
    return ok;
}

/* Highest score first; equal scores go to whoever has more wins. */
static int by_score(const void *a, const void *b)
{
    const PlayerStats *x = a, *y = b;
    if (x->score != y->score) return x->score < y->score ? 1 : -1;
    if (x->wins != y->wins) return x->wins < y->wins ? 1 : -1;
    return 0;
}

void stats_show_leaderboard(const char *path)
{
    PlayerStats *players;
    int count;
    int status = stats_load(path, &players, &count);

    printf("\n================ Leaderboard ================\n");
    if (status == -1) {
        printf("The leaderboard file is damaged.\n");
        return;
    }
    if (count == 0) {
        printf("No games played yet.\n");
        free(players);
        return;
    }

    qsort(players, (size_t)count, sizeof *players, by_score);
    printf("%-5s %-15s %6s %4s %4s %4s %7s %5s\n",
           "Rank", "Name", "Score", "W", "L", "D", "Streak", "Best");
    int shown = count < LEADERBOARD_SHOWN ? count : LEADERBOARD_SHOWN;
    for (int i = 0; i < shown; i++) {
        printf("%-5d %-15s %6d %4d %4d %4d %7d %5d\n", i + 1,
               players[i].name, (int)players[i].score, (int)players[i].wins,
               (int)players[i].losses, (int)players[i].draws,
               (int)players[i].current_streak, (int)players[i].best_streak);
    }
    if (count > shown) {
        printf("(%d more player%s)\n", count - shown,
               count - shown == 1 ? "" : "s");
    }
    printf("=============================================\n");
    free(players);
}
