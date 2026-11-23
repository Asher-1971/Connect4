#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"

int terminal_read_int(void) {
    char input[128];

    while (1) {
        if (!fgets(input, sizeof input, stdin)) {
            return -1;
        }
        if (!strchr(input, '\n')) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            if (ch == EOF) return -1;
            printf("enter a valid number: ");
            continue;
        }

        errno = 0;
        char *end;
        long value = strtol(input, &end, 10);
        if (errno == ERANGE || value < INT_MIN || value > INT_MAX ||
            end == input) {
            printf("enter a valid number: ");
            continue;
        }

        while (*end == ' ' || *end == '\t') end++;
        if (*end != '\n' && *end != '\r' && *end != '\0') {
            printf("enter a valid number: ");
            continue;
        }
        return (int)value;
    }
}

int terminal_validate_board_size(int size) {
    return size >= MIN_SIZE && size <= MAX_SIZE;
}

void terminal_init_board(GameState *game) {
    if (!game || !terminal_validate_board_size(game->rows) ||
        !terminal_validate_board_size(game->cols)) {
        return;
    }

    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            game->board[r][c] = EMPTY;
        }
    }
    game->piece_count = 0;
    game->move_count = 0;
    game->current_player = PLAYER_ONE;
    for (int i = 0; i <= MAX_PLAYERS; i++) {
        game->bomb_used[i] = false;
        game->anvil_used[i] = false;
    }
}

void terminal_print_board(const GameState *game) {
    if (!game) return;

    for (int c = 0; c < game->cols; c++) {
        printf("%d ", c + 1);
    }
    printf("\n");

    for (int r = game->rows - 1; r >= 0; r--) {
        for (int c = 0; c < game->cols; c++) {
            char piece = game->board[r][c] == P1 ? 'X'
                       : game->board[r][c] == P2 ? 'O'
                       : '.';
            printf("%c ", piece);
        }
        printf("\n");
    }
}

void terminal_clear_screen(void) {
    printf("\033[3J\033[2J\033[H");
    fflush(stdout);
}
