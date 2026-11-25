#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"
#include "rules.h"

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

    /* Two-digit column numbers need wider cells so pieces stay lined up. */
    int width = game->cols > 9 ? 3 : 2;

    for (int c = 0; c < game->cols; c++) {
        printf("%-*d", width, c + 1);
    }
    printf("\n");

    for (int r = game->rows - 1; r >= 0; r--) {
        for (int c = 0; c < game->cols; c++) {
            char piece = game->board[r][c] == P1 ? 'X'
                       : game->board[r][c] == P2 ? 'O'
                       : '.';
            printf("%-*c", width, piece);
        }
        printf("\n");
    }
}

void terminal_clear_screen(void) {
    printf("\033[3J\033[2J\033[H");
    fflush(stdout);
}

Move terminal_read_move(const GameState *game) {
    /* col -1 = input closed, -2 = save and quit, -3 = invalid, try again */
    Move closed = {-1, MOVE_NORMAL};
    Move invalid = {-3, MOVE_NORMAL};
    if (!game) return closed;

    int p = game->current_player;
    printf("Enter a column (1-%d), or 0 to save and quit: ", game->cols);
    int column = terminal_read_int();
    if (column == -1) return closed;
    if (column == 0) return (Move){-2, MOVE_NORMAL};
    if (column < 1 || column > game->cols) {
        printf("Invalid column.\n");
        return invalid;
    }

    printf("Disc: 0 normal, 1 bomb%s, 2 anvil%s: ",
           game->bomb_used[p] ? " (used)" : "",
           game->anvil_used[p] ? " (used)" : "");
    int power_type = terminal_read_int();
    if (power_type == -1) return closed;
    if (power_type < MOVE_NORMAL || power_type > MOVE_ANVIL) {
        printf("Invalid disc type.\n");
        return invalid;
    }

    Move move = {column - 1, (MoveType)power_type};
    if (!rules_is_valid_move(game, move)) {
        printf("That move isn't allowed (column full or power-up used).\n");
        return invalid;
    }
    return move;
}
