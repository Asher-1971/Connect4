#ifndef TERMINAL_H
#define TERMINAL_H

#include <stddef.h>

#include "header.h"

int terminal_read_int(void);
int terminal_validate_board_size(int size);
void terminal_init_board(GameState *game);
void terminal_print_board(const GameState *game);
void terminal_clear_screen(void);
/* Reads a name of 1 to size-1 printable characters into out; an empty line
 * gives fallback. Returns 0 if input has closed. */
int terminal_read_name(char *out, size_t size, const char *fallback);
Move terminal_read_move(const GameState *game, int undos_left);

#endif
