#ifndef SAVELOAD_H
#define SAVELOAD_H


/*
 * Save / load an unfinished game as a binary file.
 *
 * File layout:
 *   "C4SV"                      4 bytes  file ID
 *   version                     1 byte
 *   saved time                  8 bytes  seconds since 1 Jan 1970
 *   rows, cols, win_length,
 *   current_player              1 byte each
 *   move_count, piece_count     4 bytes each
 *   ai_mode, ai_difficulty      1 byte each
 *   bomb/anvil used (P1, P2)    1 byte each (4 bytes)
 *   board                       rows * cols bytes
 *   CRC-32                      4 bytes, checksum of everything above
 */


#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "header.h"

#define SAVE_SLOTS 5   /* files connect4_save1.bin ... connect4_save5.bin */

uint32_t save_crc32(const uint8_t *data, size_t len);

/* Writes the file name for slot 1..SAVE_SLOTS into path. */
void save_slot_path(int slot, char *path, size_t size);

/* Returns 1 on success, 0 on failure. */
int save_game_file(const char *path, const GameState *game, int ai_mode, int ai_difficulty);

//Returns 1 on success, 0 if there is no save file,-1 if the file is corrupted or holds an invalid game.//
int load_game_file(const char *path, GameState *game, int *ai_mode, int *ai_difficulty, time_t *saved_at);

#endif