#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "header.h"

void printBoard(GameState *game) {
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

void initBoard(GameState *game) {
    if (!game || game->rows < MIN_SIZE || game->rows > MAX_SIZE ||
        game->cols < MIN_SIZE || game->cols > MAX_SIZE) {
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

int readInt(void) {
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

int validateBoardSize(int n) {
    return n >= MIN_SIZE && n <= MAX_SIZE;
}
