#include <stdio.h>
#include <ctype.h>
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

/* Reads one line of input with surrounding spaces removed.
 * Returns a pointer into buf, or NULL if input has closed. */
static char *read_line(char *buf, size_t size) {
    while (1) {
        if (!fgets(buf, (int)size, stdin)) return NULL;
        if (strchr(buf, '\n')) break;
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        if (ch == EOF) return NULL;
        printf("Too long, try again: ");
    }
    char *start = buf;
    while (isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) *--end = '\0';
    return start;
}

int terminal_read_name(char *out, size_t size, const char *fallback) {
    char line[128];
    while (1) {
        char *input = read_line(line, sizeof line);
        if (!input) return 0;
        if (input[0] == '\0') {               /* just Enter: use the default */
            snprintf(out, size, "%s", fallback);
            return 1;
        }
        size_t len = strlen(input);
        bool ok = len < size;
        for (size_t i = 0; ok && i < len; i++) {
            if (!isprint((unsigned char)input[i])) ok = false;
        }
        if (ok) {
            memcpy(out, input, len + 1);
            return 1;
        }
        printf("Names can be up to %d letters, numbers or spaces: ",
               (int)size - 1);
    }
}

Move terminal_read_move(const GameState *game, int undos_left) {
    /* col -1 = input closed, -2 = save and quit, -3 = invalid, try again,
     * -4 = undo (only offered when undos_left > 0) */
    Move closed = {-1, MOVE_NORMAL};
    Move invalid = {-3, MOVE_NORMAL};
    if (!game) return closed;

    /* Only offer the power-ups this player still has. */
    int p = game->current_player;
    bool bomb_left = game->powerups && !game->bomb_used[p];
    bool anvil_left = game->powerups && !game->anvil_used[p];

    printf("Enter a column (1-%d)", game->cols);
    if (bomb_left) printf(", b = bomb");
    if (anvil_left) printf(", a = anvil");
    if (undos_left > 0) printf(", u = undo (%d left)", undos_left);
    printf(", 0 = save and quit: ");

    char line[128];
    char *input = read_line(line, sizeof line);
    if (!input) return closed;

    MoveType type = MOVE_NORMAL;
    int column;
    char key = (char)tolower((unsigned char)input[0]);

    if (input[0] != '\0' && input[1] == '\0' && key == 'u') {
        if (undos_left > 0) return (Move){-4, MOVE_NORMAL};
        printf("You can't undo right now.\n");
        return invalid;
    }

    if (input[0] != '\0' && input[1] == '\0' && (key == 'b' || key == 'a')) {
        /* Power-up picked first: check it's available, then ask the column. */
        if (!game->powerups) {
            printf("Power-ups are off in this game.\n");
            return invalid;
        }
        if ((key == 'b' && !bomb_left) || (key == 'a' && !anvil_left)) {
            printf("You've already used your %s.\n",
                   key == 'b' ? "bomb" : "anvil");
            return invalid;
        }
        type = (key == 'b') ? MOVE_BOMB : MOVE_ANVIL;
        printf("%s ready. Column (1-%d), or 0 to cancel: ",
               key == 'b' ? "Bomb" : "Anvil", game->cols);
        column = terminal_read_int();
        if (column == -1) return closed;
        if (column == 0) return invalid;          /* cancelled: ask again */
    } else {
        /* A plain number: a normal disc, or 0 to save. */
        char *end;
        errno = 0;
        long value = strtol(input, &end, 10);
        if (end == input || *end != '\0' || errno == ERANGE ||
            value < 0 || value > INT_MAX) {
            printf("Invalid input.\n");
            return invalid;
        }
        column = (int)value;
        if (column == 0) return (Move){-2, MOVE_NORMAL};
    }

    if (column < 1 || column > game->cols) {
        printf("Invalid column.\n");
        return invalid;
    }

    Move move = {column - 1, type};
    if (!rules_is_valid_move(game, move)) {
        /* An anvil works on any column, so only these two can fail here. */
        printf(type == MOVE_BOMB ? "A bomb needs a column that isn't full.\n"
                                 : "That column is full.\n");
        return invalid;
    }
    return move;
}
