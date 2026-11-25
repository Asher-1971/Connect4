#include <stdio.h>
#include <string.h>

#include "saveload.h"
#include "rules.h"

#define SAVE_MAGIC    "C4SV"
#define SAVE_VERSION  3
#define HEADER_SIZE   31   /* every field before the board */
#define MAX_FILE_SIZE (HEADER_SIZE + MAX_SIZE * MAX_SIZE + 4)

/* CRC-32 (the ZIP/PNG standard): long division of the data by the
 polynomial 0xEDB88320, using XOR instead of subtraction.
 The remainder is the checksum. */
uint32_t save_crc32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1u) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            }
            else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

void save_slot_path(int slot, char *path, size_t size)
{
    snprintf(path, size, "connect4_save%d.bin", slot);
}

int save_game_file(const char *path, const GameState *game, int ai_mode, int ai_difficulty)
{
    if (!path || !rules_validate_state(game)) return 0;

    uint8_t buf[MAX_FILE_SIZE];
    size_t n = 0;
    int64_t now = (int64_t)time(NULL);
    int32_t moves = game->move_count;
    int32_t pieces = game->piece_count;

    memcpy(buf + n, SAVE_MAGIC, 4);  n += 4;
    buf[n++] = SAVE_VERSION;
    memcpy(buf + n, &now, 8);        n += 8;
    buf[n++] = (uint8_t)game->rows;
    buf[n++] = (uint8_t)game->cols;
    buf[n++] = (uint8_t)game->win_length;
    buf[n++] = (uint8_t)game->current_player;
    memcpy(buf + n, &moves, 4);      n += 4;
    memcpy(buf + n, &pieces, 4);     n += 4;
    buf[n++] = (uint8_t)ai_mode;
    buf[n++] = (uint8_t)ai_difficulty;
    buf[n++] = game->bomb_used[PLAYER_ONE];
    buf[n++] = game->bomb_used[PLAYER_TWO];
    buf[n++] = game->anvil_used[PLAYER_ONE];
    buf[n++] = game->anvil_used[PLAYER_TWO];
    for (int r = 0; r < game->rows; r++)
        for (int c = 0; c < game->cols; c++)
            buf[n++] = game->board[r][c];

    uint32_t crc = save_crc32(buf, n);
    memcpy(buf + n, &crc, 4);        
    n += 4;

    /* Write a temp file first, then swap it in, so a crash mid-save
     never leaves a half-written save behind. */
    char tmp[256];
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    FILE *fp = fopen(tmp, "wb");
    if (!fp) return 0;
    size_t written = fwrite(buf, 1, n, fp);
    if (fclose(fp) != 0 || written != n) {
        remove(tmp);
        return 0;
    }
    remove(path);                 /* Windows rename() won't overwrite */
    return rename(tmp, path) == 0;
}

int load_game_file(const char *path, GameState *game,
                   int *ai_mode, int *ai_difficulty, time_t *saved_at)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    uint8_t buf[MAX_FILE_SIZE + 1];  /* +1 to notice extra bytes */
    size_t len = fread(buf, 1, sizeof buf, fp);
    fclose(fp);

  /* Check ID, version and CRC before trusting anything else. */
    if (len < HEADER_SIZE + 4 || len > MAX_FILE_SIZE ||
        memcmp(buf, SAVE_MAGIC, 4) != 0 || buf[4] != SAVE_VERSION)
        return -1;
    uint32_t stored_crc;
    memcpy(&stored_crc, buf + len - 4, 4);
    if (save_crc32(buf, len - 4) != stored_crc) return -1;
    
    GameState g = {0};
    int64_t time_saved;
    int32_t moves, pieces;
    size_t n = 5;
    memcpy(&time_saved, buf + n, 8); n += 8;
    g.rows = buf[n++];
    g.cols = buf[n++];
    g.win_length = buf[n++];
    g.current_player = buf[n++];
    memcpy(&moves, buf + n, 4);      n += 4;
    memcpy(&pieces, buf + n, 4);     n += 4;
    int mode = buf[n++];
    int difficulty = buf[n++];
    g.bomb_used[PLAYER_ONE] = buf[n++] != 0;
    g.bomb_used[PLAYER_TWO] = buf[n++] != 0;
    g.anvil_used[PLAYER_ONE] = buf[n++] != 0;
    g.anvil_used[PLAYER_TWO] = buf[n++] != 0;
    g.move_count = moves;
    g.piece_count = pieces;

    if (g.rows < MIN_SIZE || g.rows > MAX_SIZE ||
        g.cols < MIN_SIZE || g.cols > MAX_SIZE ||
        len != (size_t)(HEADER_SIZE + g.rows * g.cols + 4) ||
        mode < 1 || mode > 2 || difficulty < 0 || difficulty > 3)
        return -1;
    for (int r = 0; r < g.rows; r++)
        for (int c = 0; c < g.cols; c++)
            g.board[r][c] = buf[n++];
}