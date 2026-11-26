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
 *   power-ups on                1 byte (0 or 1)
 *   undos used (P1, P2)         1 byte each
 *   names (P1, P2)             16 bytes each, zero padded
 *   board                     rows * cols bytes
 *   move list               move_count * 2 bytes (column, type)
 *   CRC-32                      4 bytes, checksum of everything above
 *
 * On load the move list is replayed from an empty board and must give
 * exactly the saved board, so undo keeps working after resuming.
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
int save_game_file(const char *path, const GameState *game,
                   const MoveHistory *history,
                   const char *name1, const char *name2,
                   int ai_mode, int ai_difficulty);

//Returns 1 on success, 0 if there is no save file,-1 if the file is corrupted or holds an invalid game.//
/* history and names may be NULL when only the game itself is needed. */
int load_game_file(const char *path, GameState *game, MoveHistory *history,
                   char names[MAX_PLAYERS + 1][NAME_LEN], int *ai_mode, int *ai_difficulty, time_t *saved_at);

#endif