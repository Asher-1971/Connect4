#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "module3.h"
#include "rules.h"
#include "saveload.h"
#include "terminal.h"

int saveGame(const GameState *game, const GameConfig *config)
{
    if (!game || !config) return 0;

    /* A resumed game goes back into its own slot. A new game takes an
     * empty (or unreadable) slot, otherwise it replaces the oldest save. */
    int slot = config->save_slot;
    if (slot < 1 || slot > SAVE_SLOTS) {
        time_t oldest = 0;
        slot = 0;
        for (int s = 1; s <= SAVE_SLOTS; s++) {
            char path[64];
            GameState tmp;
            int mode, diff;
            time_t when;
            save_slot_path(s, path, sizeof path);
            if (load_game_file(path, &tmp, &mode, &diff, &when) != 1) {
                slot = s;
                break;
            }
            if (slot == 0 || when < oldest) {
                slot = s;
                oldest = when;
            }
        }
    }

    char path[64];
    save_slot_path(slot, path, sizeof path);
    if (!save_game_file(path, game, config->ai_mode, config->ai_difficulty)) {
        printf("Save failed.\n");
        return 0;
    }
    printf("Game saved to slot %d.\n", slot);
    return 1;
}

typedef struct {
    GameState game;
    int ai_mode;
    int ai_difficulty;
    int slot;
    time_t saved_at;
} SaveInfo;

int loadGame(GameState *game, GameConfig *config)
{
    SaveInfo saves[SAVE_SLOTS];
    int count = 0;

    /* Read every slot, keeping the list sorted newest first. */
    for (int s = 1; s <= SAVE_SLOTS; s++) {
        char path[64];
        SaveInfo info;
        save_slot_path(s, path, sizeof path);
        if (load_game_file(path, &info.game, &info.ai_mode,
                           &info.ai_difficulty, &info.saved_at) != 1) {
            continue;   /* empty or corrupted slot */
        }
        info.slot = s;
        int i = count++;
        while (i > 0 && saves[i - 1].saved_at < info.saved_at) {
            saves[i] = saves[i - 1];
            i--;
        }
        saves[i] = info;
    }

    if (count == 0) {
        printf("No saved games found.\n");
        return 0;
    }

    printf("\nSaved games (newest first):\n");
    for (int i = 0; i < count; i++) {
        char when[64] = "unknown time";
        struct tm *local = localtime(&saves[i].saved_at);
        if (local) strftime(when, sizeof when, "%d %b %Y, %H:%M", local);
        printf("%d. %s  (%dx%d, %s%s, %d moves)\n", i + 1, when,
               saves[i].game.rows, saves[i].game.cols,
               saves[i].ai_mode == 2 ? "Human vs AI" : "Human vs Human",
               saves[i].game.powerups ? ", power-ups" : "",
               saves[i].game.move_count);
    }
    printf("0. Start a new game\nEnter your choice: ");

    int choice = terminal_read_int();
    while (choice < 0 || choice > count) {
        if (choice == -1) return 0;
        printf("Enter 0-%d: ", count);
        choice = terminal_read_int();
    }
    if (choice == 0) return 0;

    SaveInfo *picked = &saves[choice - 1];
    *game = picked->game;
    config->ai_mode = picked->ai_mode;
    config->ai_difficulty = picked->ai_difficulty;
    config->save_slot = picked->slot;
    config->win = false;
    config->tie = false;
    printf("\n");
    return 1;
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
        if (loadGame(game, config)) {
            return;
        }
        printf("Starting a new game.\n");
        config->menu_choice = 1;
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
        game->win_length = 4;   /* classic rules, no need to ask */
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
    }

    printf("\nPower-ups:\n");
    printf("1. Off (classic rules)\n");
    printf("2. On (one bomb and one anvil each)\n");
    printf("Enter your choice: ");

    int powerChoice = terminal_read_int();
    while (powerChoice < 1 || powerChoice > 2) {
        if (powerChoice == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        powerChoice = terminal_read_int();
    }
    game->powerups = (powerChoice == 2);

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

/* Applies a move and writes a sentence about it into msg, so it can be shown
 * under the board after the screen is cleared. Power-ups stand out with ***.
 * Returns false (and changes nothing) if the move isn't allowed. */
static bool playMove(GameState *game, Move move, const char *who,
                     char *msg, size_t size)
{
    GameState before = *game;
    if (move.col < 0 || !rules_apply_move(game, move, game)) {
        return false;
    }

    if (move.type == MOVE_NORMAL) {
        snprintf(msg, size, "%s dropped a disc in column %d.",
                 who, move.col + 1);
    } else {
        int cleared = before.piece_count - game->piece_count;
        snprintf(msg, size,
                 "*** %s used %s on column %d and cleared %d disc%s! ***",
                 who, move.type == MOVE_BOMB ? "a BOMB" : "an ANVIL",
                 move.col + 1, cleared, cleared == 1 ? "" : "s");
    }
    return true;
}

void uiHumanvsHuman(GameState *game, GameConfig *config)
{
    if (!game || !config) {
        return;
    }

    char last[128] = "";   /* what happened on the previous move */

    while (!config->win && !config->tie) {
        terminal_print_board(game);
        if (last[0]) printf("%s\n", last);

        int winner = rules_check_winner(game);
        if (winner == 3) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        } else if (winner != 0) {
            printf("Player %d (%c) wins!\n", winner, winner == P1 ? 'X' : 'O');
            config->win = true;
            break;
        }
        if (rules_is_draw(game)) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        }

        char who[32];
        snprintf(who, sizeof who, "Player %d (%c)", game->current_player,
                 game->current_player == PLAYER_ONE ? 'X' : 'O');
        printf("%s turn:\n", who);

        Move move = terminal_read_move(game);
        if (move.col == -1) return;              /* input closed */
        if (move.col == -2) {                    /* 0 = save and quit */
            if (!saveGame(game, config)) continue;
            printf("Exiting...\n");
            return;
        }
        if (!playMove(game, move, who, last, sizeof last)) continue;

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

    char last[128] = "";   /* what happened on the previous move */

    while (!config->win && !config->tie) {
        terminal_print_board(game);
        if (last[0]) printf("%s\n", last);

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
            Move move = terminal_read_move(game);
            if (move.col == -1) return;          /* input closed */
            if (move.col == -2) {                /* 0 = save and quit */
                if (!saveGame(game, config)) continue;
                printf("Exiting...\n");
                return;
            }
            if (!playMove(game, move, "You", last, sizeof last)) continue;
        } else {
            printf("AI turn (O):\n");

            if (difficulty == AI_EASY) {
                Move valid[MAX_DIM * 3];
                int validCount = 0;
                for (int c = 0; c < game->cols; c++) {
                    Move candidates[] = {
                        {c, MOVE_NORMAL},
                        {c, MOVE_BOMB},
                        {c, MOVE_ANVIL}
                    };
                    for (int t = 0; t < 3; t++) {
                        if (rules_is_valid_move(game, candidates[t])) {
                            valid[validCount++] = candidates[t];
                        }
                    }
                }
                Move move = validCount > 0
                    ? valid[rand() % validCount]
                    : (Move){-1, MOVE_NORMAL};
                if (!playMove(game, move, "AI", last, sizeof last)) continue;
            } else {
                int timeBudget = (difficulty == AI_NORMAL) ? 100 : 1000;
                Move move = find_best_move(game, timeBudget);
                if (!playMove(game, move, "AI", last, sizeof last)) continue;
            }
        }

        terminal_clear_screen();
    }
}

int main(void)
{
    while (1) {
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

        /* Saved and quit (or input closed): stop here. */
        if (!config.win && !config.tie) {
            return 0;
        }

        /* A resumed game that has now finished no longer needs its save. */
        if (config.save_slot > 0) {
            char path[64];
            save_slot_path(config.save_slot, path, sizeof path);
            remove(path);
        }

        printf("\nPlay again?\n1. Yes\n2. No\nEnter your choice: ");
        int again = terminal_read_int();
        while (again != 1 && again != 2) {
            if (again == -1) return 0;
            printf("Enter 1 or 2: ");
            again = terminal_read_int();
        }
        if (again == 2) {
            return 0;
        }
        printf("\n");
    }
}
