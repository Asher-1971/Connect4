#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "rules.h"
#include "saveload.h"
#include "terminal.h"

#define TEST_FILE "test_saveload.bin"

int main()
{
  /* CRC-32 gives the official check value. */
    assert(save_crc32((const uint8_t *)"123456789", 9) == 0xCBF43926u);

    GameState g = {0}, out = {0};
    g.rows = 6; g.cols = 7; g.win_length = 4;
    terminal_init_board(&g);
    int cols[] = {3, 3, 0, 6, 2};
    for (int i = 0; i < 5; i++)
        assert(rules_apply_move(&g, (Move){cols[i], MOVE_NORMAL}, &g));

      /* Save then load gives back the same game, mode and a recent time. */
    int mode = 0, diff = 0;
    time_t when = 0;
    assert(save_game_file(TEST_FILE, &g, 2, 3));
    assert(load_game_file(TEST_FILE, &out, &mode, &diff, &when) == 1);
    assert(memcmp(g.board, out.board, sizeof g.board) == 0);
    assert(out.rows == 6 && out.cols == 7 && out.win_length == 4);
    assert(out.current_player == g.current_player);
    assert(out.move_count == 5 && out.piece_count == 5);
    assert(mode == 2 && diff == 3);
    assert(when > 0 && time(NULL) - when < 60);

    /* Changing any single byte makes the load fail. */
    uint8_t buf[512];
    FILE *fp = fopen(TEST_FILE, "rb");
    size_t len = fread(buf, 1, sizeof buf, fp);
    fclose(fp);
    for (size_t i = 0; i < len; i++) 
    {
        buf[i] ^= 0x01;
        fp = fopen(TEST_FILE, "wb");
        fwrite(buf, 1, len, fp);
        fclose(fp);
        assert(load_game_file(TEST_FILE, &out, &mode, &diff, &when) == -1);
        buf[i] ^= 0x01;
    }

     /* Missing file. */
    remove(TEST_FILE);
    assert(load_game_file(TEST_FILE, &out, &mode, &diff, &when) == 0);

    printf("All save/load tests passed.\n");
    return 0;
}