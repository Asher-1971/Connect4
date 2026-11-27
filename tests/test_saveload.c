#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "rules.h"
#include "saveload.h"
#include "terminal.h"

#define TEST_FILE "test_saveload.bin"

/* Plays a move and records it, the way game.c does. */
static void play(GameState *g, MoveHistory *h, Move m)
{
    assert(rules_apply_move(g, m, g));
    h->moves[h->count++] = m;
}

int main()
{
    /* CRC-32 gives the official check value. */
    assert(save_crc32((const uint8_t *)"123456789", 9) == 0xCBF43926u);

    GameState g = {0}, out = {0};
    MoveHistory h = {0}, hout = {0};
    char names[3][NAME_LEN] = {"", "Ali", "Sara"}, nout[3][NAME_LEN];
    g.rows = 6; g.cols = 7; g.win_length = 4;
    terminal_init_board(&g);
    int cols[] = {3, 3, 0, 6, 2};
    for (int i = 0; i < 5; i++)
        play(&g, &h, (Move){cols[i], MOVE_NORMAL});
    h.undos_used[PLAYER_ONE] = 1;

    /* Save then load gives back the same game, history, mode and time. */
    int mode = 0, diff = 0;
    time_t when = 0;
    assert(save_game_file(TEST_FILE, &g, &h, names[1], names[2], 2, 3));
    assert(load_game_file(TEST_FILE, &out, &hout, nout, &mode, &diff, &when) == 1);
    assert(memcmp(g.board, out.board, sizeof g.board) == 0);
    assert(out.rows == 6 && out.cols == 7 && out.win_length == 4);
    assert(out.current_player == g.current_player);
    assert(out.move_count == 5 && out.piece_count == 5);
    assert(mode == 2 && diff == 3);
    assert(when > 0 && time(NULL) - when < 60);
    assert(out.powerups == false);
    assert(hout.count == 5 && hout.undos_used[PLAYER_ONE] == 1);
    assert(strcmp(nout[1], "Ali") == 0 && strcmp(nout[2], "Sara") == 0);
    for (int i = 0; i < 5; i++)
        assert(hout.moves[i].col == cols[i]);

    /* history can be NULL when only the game is wanted. */
    assert(load_game_file(TEST_FILE, &out, NULL, NULL, &mode, &diff, &when) == 1);

    /* Undo = replay without the last two moves. */
    GameState undone;
    assert(rules_replay(&g, h.moves, 3, &undone));
    assert(undone.move_count == 3 && undone.piece_count == 3);
    assert(undone.current_player == PLAYER_TWO);
    assert(undone.board[0][6] == EMPTY && undone.board[0][2] == EMPTY);
    assert(undone.board[0][0] == P1);

    /* Undo limit grows with the board. */
    assert(rules_undo_limit(&g) == 2);
    GameState small = g;  small.rows = 4; small.cols = 4;
    assert(rules_undo_limit(&small) == 1);
    GameState big = g;  big.rows = 20; big.cols = 20;
    assert(rules_undo_limit(&big) == 25);

    /* A history that doesn't match the board is refused when saving. */
    MoveHistory short_h = h;
    short_h.count = 4;
    assert(!save_game_file(TEST_FILE, &g, &short_h, names[1], names[2], 2, 3));

    /* An empty name is refused too. */
    char no_name[3][NAME_LEN] = {"", "", "Sara"};
    assert(!save_game_file(TEST_FILE, &g, &h, no_name[1], no_name[2], 2, 3));

    /* A power-ups game keeps its setting, used power-ups and history. */
    GameState pg = {0}, pout = {0};
    MoveHistory ph = {0};
    pg.rows = 6; pg.cols = 7; pg.win_length = 4; pg.powerups = true;
    terminal_init_board(&pg);
    play(&pg, &ph, (Move){3, MOVE_NORMAL});
    play(&pg, &ph, (Move){3, MOVE_ANVIL});
    assert(save_game_file(TEST_FILE, &pg, &ph, names[1], names[2], 1, 0));
    assert(load_game_file(TEST_FILE, &pout, &hout, nout, &mode, &diff, &when) == 1);
    assert(pout.powerups == true && pout.anvil_used[PLAYER_TWO] == true);
    assert(pout.move_count == 2 && pout.piece_count == 0);
    assert(hout.count == 2 && hout.moves[1].type == MOVE_ANVIL);
    assert(save_game_file(TEST_FILE, &g, &h, names[1], names[2], 2, 3));   /* back to the first game */

    /* Changing any single byte makes the load fail. */
    uint8_t buf[1024];
    FILE *fp = fopen(TEST_FILE, "rb");
    size_t len = fread(buf, 1, sizeof buf, fp);
    fclose(fp);
    for (size_t i = 0; i < len; i++)
    {
        buf[i] ^= 0x01;
        fp = fopen(TEST_FILE, "wb");
        fwrite(buf, 1, len, fp);
        fclose(fp);
        assert(load_game_file(TEST_FILE, &out, &hout, nout, &mode, &diff, &when) == -1);
        buf[i] ^= 0x01;
    }

    /* A move list that doesn't replay to the saved board is rejected even
     * with a correct CRC: swap moves 1 and 3 and fix the checksum. */
    size_t moves_at = len - 4 - 5 * 2;   /* the move list is just before the CRC */
    uint8_t tmp = buf[moves_at];
    buf[moves_at] = buf[moves_at + 4];
    buf[moves_at + 4] = tmp;
    uint32_t crc = save_crc32(buf, len - 4);
    memcpy(buf + len - 4, &crc, 4);
    fp = fopen(TEST_FILE, "wb");
    fwrite(buf, 1, len, fp);
    fclose(fp);
    assert(load_game_file(TEST_FILE, &out, &hout, nout, &mode, &diff, &when) == -1);

    /* Missing file. */
    remove(TEST_FILE);
    assert(load_game_file(TEST_FILE, &out, &hout, nout, &mode, &diff, &when) == 0);

    printf("All save/load tests passed.\n");
    return 0;
}
