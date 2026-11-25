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
    
}