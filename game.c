#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "module3.h"
#include "rules.h"
#include "terminal.h"

int saveGame(GameState *game)
{
    (void)game;
    printf("Save/load is not implemented yet.\n");
    return 0;
}

int loadGame(GameState *game)
{
    (void)game;
    printf("No saved game is available; save/load is not implemented yet.\n");
    return 0;
}

void gameInputs(GameState *game, GameConfig *config)
{
    if (!game || !config) {
        return;
    }

    printf("Connect 4 Game\n");
    printf("1. New Game\n");
    printf("2. Resume Game\n");
    printf("Enter your choice: ");

    config->menu_choice = terminal_read_int();
    while (config->menu_choice < 1 || config->menu_choice > 2) {
        if (config->menu_choice == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        config->menu_choice = terminal_read_int();
    }

    if (config->menu_choice == 2) {
        if (!loadGame(game)) {
            config->menu_choice = 0;
        }
        return;
    }

    printf("\nBoard size:\n");
    printf("1. Classic 6x7\n");
    printf("2. Custom size\n");
    printf("Enter your choice: ");

    int sizeChoice = terminal_read_int();
    while (sizeChoice < 1 || sizeChoice > 2) {
        if (sizeChoice == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        sizeChoice = terminal_read_int();
    }

    if (sizeChoice == 1) {
        game->rows = 6;
        game->cols = 7;
    } else {
        printf("Rows (%d-%d): ", MIN_SIZE, MAX_SIZE);
        game->rows = terminal_read_int();
        while (!terminal_validate_board_size(game->rows)) {
            if (game->rows == -1) {
                return;
            }
            printf("Invalid rows. Enter %d-%d: ", MIN_SIZE, MAX_SIZE);
            game->rows = terminal_read_int();
        }

        printf("Columns (%d-%d): ", MIN_SIZE, MAX_SIZE);
        game->cols = terminal_read_int();
        while (!terminal_validate_board_size(game->cols)) {
            if (game->cols == -1) {
                return;
            }
            printf("Invalid columns. Enter %d-%d: ", MIN_SIZE, MAX_SIZE);
            game->cols = terminal_read_int();
        }
    }

    printf("Win length (4 or 5): ");
    game->win_length = terminal_read_int();
    while (game->win_length < 4 || game->win_length > 5 ||
           (game->win_length > game->rows && game->win_length > game->cols)) {
        if (game->win_length == -1) {
            return;
        }
        printf("Invalid win length. Enter 4 or 5: ");
        game->win_length = terminal_read_int();
    }

    printf("\nChoose game mode:\n");
    printf("1. Human vs Human\n");
    printf("2. Human vs AI\n");
    printf("Enter your choice: ");

    config->ai_mode = terminal_read_int();
    while (config->ai_mode < 1 || config->ai_mode > 2) {
        if (config->ai_mode == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        config->ai_mode = terminal_read_int();
    }

    if (config->ai_mode == 2) {
        printf("\nChoose AI difficulty:\n");
        printf("1. Easy\n");
        printf("2. Normal\n");
        printf("3. Hard\n");
        printf("Enter difficulty: ");

        config->ai_difficulty = terminal_read_int();
        while (config->ai_difficulty < AI_EASY || config->ai_difficulty > AI_HARD) {
            if (config->ai_difficulty == -1) {
                return;
            }
            printf("Enter 1, 2, or 3: ");
            config->ai_difficulty = terminal_read_int();
        }
    }

    config->win = false;
    config->tie = false;
    game->move_count = 0;
    game->piece_count = 0;
    game->current_player = PLAYER_ONE;
    terminal_init_board(game);
}

static int promptColumn(GameState *game)
{
    if (!game) {
        return -1;
    }

    printf("Enter a column (1-%d), or 0 to save and quit: ", game->cols);
    int column = terminal_read_int();
    if (column == -1) {
        return -1;
    }

    while (column < 0 || column > game->cols ||
           (column > 0 &&
            !rules_is_valid_move(game,
                                 (Move){column - 1, MOVE_NORMAL}))) {
        printf("Invalid or full column. Enter 0-%d: ", game->cols);
        column = terminal_read_int();
        if (column == -1) {
            return -1;
        }
    }

    return column;
}

void uiHumanvsHuman(GameState *game, GameConfig *config)
{
    if (!game || !config) {
        return;
    }

    while (!config->win && !config->tie) {
        terminal_print_board(game);

        int winner = rules_check_winner(game);
        if (winner == 3) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        } else if (winner != 0) {
            char winner_piece = (winner == P1) ? 'X' : 'O';
            printf("Player %c wins!\n", winner_piece);
            config->win = true;
            break;
        }
        if (rules_is_draw(game)) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        }

        printf("Player %d turn:\n", game->current_player);

        int column = promptColumn(game);
        if (column == -1) {
            return;
        }
        if (column == 0) {
            saveGame(game);
            printf("Exiting...\n");
            return;
        }

        Move move = {column - 1, MOVE_NORMAL};
        if (!rules_apply_move(game, move, game)) {
            continue;
        }

        terminal_clear_screen();
    }
}

void uiHumanvsAI(GameState *game, GameConfig *config)
{
    if (!game || !config) {
        return;
    }

    int difficulty = config->ai_difficulty;
    if (difficulty < AI_EASY || difficulty > AI_HARD) {
        difficulty = AI_NORMAL;
    }

    if (difficulty == AI_EASY) {
        srand((unsigned int)time(NULL));
    }

    while (!config->win && !config->tie) {
        terminal_print_board(game);

        int winner = rules_check_winner(game);
        if (winner == 3 || rules_is_draw(game)) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        } else if (winner == P1) {
            printf("You win!\n");
            config->win = true;
            break;
        } else if (winner == P2) {
            printf("AI wins!\n");
            config->win = true;
            break;
        }

        if (game->current_player == PLAYER_ONE) {
            printf("Your turn (X):\n");
            int column = promptColumn(game);
            if (column == -1) {
                return;
            }
            if (column == 0) {
                saveGame(game);
                printf("Exiting...\n");
                return;
            }
            Move move = {column - 1, MOVE_NORMAL};
            if (!rules_apply_move(game, move, game)) continue;
        } else {
            printf("AI turn (O):\n");

            if (difficulty == AI_EASY) {
                int valid[20];
                int validCount = 0;
                for (int c = 0; c < game->cols; c++) {
                    if (rules_is_valid_move(game,
                                           (Move){c, MOVE_NORMAL})) {
                        valid[validCount++] = c + 1;
                    }
                }
                int column = validCount > 0
                    ? valid[rand() % validCount]
                    : -1;
                Move move = {column - 1, MOVE_NORMAL};
                if (column > 0 && !rules_apply_move(game, move, game)) continue;
            } else {
                int timeBudget = (difficulty == AI_NORMAL) ? 100 : 1000;
                Move move = find_best_move(game, timeBudget);
                if (!rules_apply_move(game, move, game)) continue;
            }
        }

        terminal_clear_screen();
    }
}

int main(void)
{
    GameState game = {0};
    GameConfig config = {0};
    gameInputs(&game, &config);

    if (config.menu_choice == 0) {
        return 0;
    }

    if (config.ai_mode == 1) {
        uiHumanvsHuman(&game, &config);
    } else if (config.ai_mode == 2) {
        uiHumanvsAI(&game, &config);
    }

    return 0;
}
