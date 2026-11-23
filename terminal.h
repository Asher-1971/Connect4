#ifndef TERMINAL_H
#define TERMINAL_H

#include "header.h"

int terminal_read_int(void);
int terminal_validate_board_size(int size);
void terminal_init_board(GameState *game);
void terminal_print_board(const GameState *game);
void terminal_clear_screen(void);
Move terminal_read_move(const GameState *game);

#endif
