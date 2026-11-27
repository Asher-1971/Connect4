#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stats.h"

#define TEST_FILE "test_stats.bin"

int main(void)
{
    remove(TEST_FILE);

    /* Points: the example from the plan, Hard AI on 6x7 in 21 moves. */
    GameState g = {0};
    g.rows = 6; g.cols = 7; g.move_count = 21;
    char why[96];
    assert(stats_win_points(&g, 3, 0, why, sizeof why) == 363);
    assert(strstr(why, "= 363") != NULL);
    assert(stats_win_points(&g, 1, 0, NULL, 0) == 121);
    assert(stats_win_points(&g, 1, 2, NULL, 0) == 101);      /* -10 per undo */
    assert(stats_win_points(&g, 1, 50, NULL, 0) == MIN_WIN_POINTS);
    GameState big = g;  big.rows = 10; big.cols = 10; big.move_count = 30;
    assert(stats_win_points(&big, 1, 0, NULL, 0) == 100 + 58 + 70);

    /* Names ignore capitals. */
    assert(stats_same_name("Ali", "aLI"));
    assert(!stats_same_name("Ali", "Alia"));
    assert(!stats_same_name("Ali", "Al"));

    /* No file yet: an empty list. */
    PlayerStats *list;
    int count;
    assert(stats_load(TEST_FILE, &list, &count) == 0 && count == 0);

    /* Streaks: win, win, draw keeps it, loss resets it, best stays. */
    PlayerStats ps;
    assert(stats_record(TEST_FILE, "Ali", RESULT_WIN, 100, &ps));
    assert(ps.wins == 1 && ps.current_streak == 1 && ps.best_streak == 1);
    assert(stats_record(TEST_FILE, "ali", RESULT_WIN, 50, &ps));
    assert(ps.current_streak == 2 && ps.best_streak == 2 && ps.score == 150);
    assert(stats_record(TEST_FILE, "ALI", RESULT_DRAW, DRAW_POINTS, &ps));
    assert(ps.current_streak == 2 && ps.draws == 1 && ps.score == 170);
    assert(stats_record(TEST_FILE, "Ali", RESULT_LOSS, 0, &ps));
    assert(ps.current_streak == 0 && ps.best_streak == 2 && ps.losses == 1);
    assert(strcmp(ps.name, "Ali") == 0);   /* first spelling is kept */

    /* A second player is added, not merged. */
    assert(stats_record(TEST_FILE, "Sara", RESULT_WIN, 400, &ps));
    assert(stats_load(TEST_FILE, &list, &count) == 1 && count == 2);
    assert(strcmp(list[1].name, "Sara") == 0 && list[1].score == 400);
    free(list);

    /* Changing any byte makes the file count as damaged. */
    uint8_t buf[256];
    FILE *fp = fopen(TEST_FILE, "rb");
    size_t len = fread(buf, 1, sizeof buf, fp);
    fclose(fp);
    assert(len == 9 + 2 * 40 + 4);
    for (size_t i = 0; i < len; i++) {
        buf[i] ^= 0x01;
        fp = fopen(TEST_FILE, "wb");
        fwrite(buf, 1, len, fp);
        fclose(fp);
        assert(stats_load(TEST_FILE, &list, &count) == -1 && list == NULL);
        buf[i] ^= 0x01;
    }

    /* Many players: the list grows with realloc and still loads. */
    remove(TEST_FILE);
    for (int i = 0; i < 300; i++) {
        char name[NAME_LEN];
        snprintf(name, sizeof name, "P%d", i);
        assert(stats_record(TEST_FILE, name, RESULT_WIN, i, NULL));
    }
    assert(stats_load(TEST_FILE, &list, &count) == 1 && count == 300);
    assert(list[299].score == 299);
    free(list);

    stats_show_leaderboard(TEST_FILE);
    remove(TEST_FILE);
    printf("All stats tests passed.\n");
    return 0;
}
