#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
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
            if (load_game_file(path, &tmp, NULL, NULL, &mode, &diff, &when) != 1) {
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
    if (!save_game_file(path, game, &config->history,
                        config->names[PLAYER_ONE], config->names[PLAYER_TWO],
                        config->ai_mode, config->ai_difficulty)) {
        printf("Save failed.\n");
        return 0;
    }
    printf("Game saved to slot %d.\n", slot);
    return 1;
}

typedef struct {
    GameState game;
    MoveHistory history;
    char names[MAX_PLAYERS + 1][NAME_LEN];
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
        if (load_game_file(path, &info.game, &info.history, info.names,
                           &info.ai_mode, &info.ai_difficulty, &info.saved_at) != 1) {
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
        printf("%d. %s  %s vs %s  (%dx%d%s, %d move%s)\n", i + 1, when,
               saves[i].names[PLAYER_ONE], saves[i].names[PLAYER_TWO],
               saves[i].game.rows, saves[i].game.cols,
               saves[i].game.powerups ? ", power-ups" : "",
               saves[i].game.move_count,
               saves[i].game.move_count == 1 ? "" : "s");
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
    config->history = picked->history;
    memcpy(config->names, picked->names, sizeof config->names);
    config->win = false;
    config->tie = false;
    printf("\n");
    return 1;
}

/* Names match without caring about capitals, so "Ali" and "ali" are one player. */
static bool sameName(const char *a, const char *b)
{
    while (*a && tolower((unsigned char)*a) == tolower((unsigned char)*b)) {
        a++;
        b++;
    }
    return tolower((unsigned char)*a) == tolower((unsigned char)*b);
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

    /* Names are shown in prompts and in the saved games list. */
    if (config->ai_mode == 1) {
        printf("\nPlayer 1 (X) name (Enter for \"Player 1\"): ");
        if (!terminal_read_name(config->names[PLAYER_ONE], NAME_LEN,
                                "Player 1")) {
            config->ai_mode = 0;
            return;
        }
        printf("Player 2 (O) name (Enter for \"Player 2\"): ");
        while (1) {
            if (!terminal_read_name(config->names[PLAYER_TWO], NAME_LEN,
                                    "Player 2")) {
                config->ai_mode = 0;
                return;
            }
            if (!sameName(config->names[PLAYER_ONE],
                          config->names[PLAYER_TWO])) {
                break;
            }
            printf("Pick a different name from Player 1: ");
        }
    } else {
        printf("\nYour name (Enter for \"Player\"): ");
        if (!terminal_read_name(config->names[PLAYER_ONE], NAME_LEN,
                                "Player")) {
            config->ai_mode = 0;
            return;
        }
        snprintf(config->names[PLAYER_TWO], NAME_LEN, "Computer");
    }

    config->win = false;
    config->tie = false;
    game->move_count = 0;
    game->piece_count = 0;
    game->current_player = PLAYER_ONE;
    terminal_init_board(game);
    config->history = (MoveHistory){0};
}

/* Applies a move and writes a sentence about it into msg, so it can be shown
 * under the board after the screen is cleared. Power-ups stand out with ***.
 * Returns false (and changes nothing) if the move isn't allowed. */
static bool playMove(GameState *game, MoveHistory *history, Move move,
                     const char *who, char *msg, size_t size)
{
    GameState before = *game;
    if (move.col < 0 || history->count >= MAX_MOVES ||
        !rules_apply_move(game, move, game)) {
        return false;
    }
    history->moves[history->count++] = move;

    if (move.type == MOVE_NORMAL) {
        snprintf(msg, size, "%s dropped a disc in column %d.",
                 who, move.col + 1);
    } else {
        int cleared = before.piece_count - game->piece_count;
        const char *name = move.type == MOVE_BOMB ? "BOMB" : "ANVIL";
        snprintf(msg, size,
                 "*** %s used %s on column %d and cleared %d disc%s! ***",
                 who, move.type == MOVE_BOMB ? "a BOMB" : "an ANVIL",
                 move.col + 1, cleared, cleared == 1 ? "" : "s");

        /* The board before the power-up is still on screen, so print a
         * banner and the board after it, then wait so it can't be missed. */
        printf("\n==========================================\n");
        printf("  %s used %s on column %d!\n", who,
               move.type == MOVE_BOMB ? "a BOMB" : "an ANVIL", move.col + 1);
        printf("  %d disc%s cleared.\n", cleared, cleared == 1 ? "" : "s");
        printf("==========================================\n");
        printf("Board after the %s:\n", name);
        terminal_print_board(game);
        printf("Press Enter to continue...");
        fflush(stdout);
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
    return true;
}

/* Undos the current player can still use now; 0 hides the option.
 * An undo needs two moves to take back: your own and the reply to it. */
static int undosLeft(const GameState *game, const MoveHistory *history)
{
    if (history->count < 2) return 0;
    int left = rules_undo_limit(game) - history->undos_used[game->current_player];
    return left > 0 ? left : 0;
}

/* Takes back the current player's last move and the reply after it, so it is
 * their turn again in the position before their move. Power-ups can't be
 * taken back. Returns false (and changes nothing) if the undo isn't allowed. */
static bool undoMove(GameState *game, MoveHistory *history, const char *who,
                     char *msg, size_t size)
{
    int n = history->count;
    if (history->moves[n - 1].type != MOVE_NORMAL ||
        history->moves[n - 2].type != MOVE_NORMAL) {
        printf("You can't undo a power-up.\n");
        return false;
    }

    GameState replayed;
    if (!rules_replay(game, history->moves, n - 2, &replayed)) {
        printf("Undo failed.\n");
        return false;
    }
    *game = replayed;
    history->count = n - 2;
    history->undos_used[game->current_player]++;

    int left = rules_undo_limit(game) - history->undos_used[game->current_player];
    snprintf(msg, size, "%s took back a move (%d undo%s left).",
             who, left, left == 1 ? "" : "s");
    return true;
}

/* "Ali (X)": the name and disc shown in prompts and messages. */
static void playerLabel(const GameConfig *config, int player,
                        char *out, size_t size)
{
    snprintf(out, size, "%s (%c)", config->names[player],
             player == PLAYER_ONE ? 'X' : 'O');
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
            char winnerName[32];
            playerLabel(config, winner, winnerName, sizeof winnerName);
            printf("%s wins!\n", winnerName);
            config->win = true;
            break;
        }
        if (rules_is_draw(game)) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        }

        char who[32];
        playerLabel(config, game->current_player, who, sizeof who);
        printf("%s turn:\n", who);

        Move move = terminal_read_move(game, undosLeft(game, &config->history));
        if (move.col == -1) return;              /* input closed */
        if (move.col == -2) {                    /* 0 = save and quit */
            if (!saveGame(game, config)) continue;
            printf("Exiting...\n");
            return;
        }
        if (move.col == -4) {                    /* u = undo */
            if (undoMove(game, &config->history, who, last, sizeof last)) {
                terminal_clear_screen();
            }
            continue;
        }
        if (!playMove(game, &config->history, move, who, last, sizeof last)) continue;

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
    char me[32], ai[32];
    playerLabel(config, PLAYER_ONE, me, sizeof me);
    playerLabel(config, PLAYER_TWO, ai, sizeof ai);

    while (!config->win && !config->tie) {
        terminal_print_board(game);
        if (last[0]) printf("%s\n", last);

        int winner = rules_check_winner(game);
        if (winner == 3 || rules_is_draw(game)) {
            printf("It's a draw!\n");
            config->tie = true;
            break;
        } else if (winner == P1) {
            printf("%s wins!\n", me);
            config->win = true;
            break;
        } else if (winner == P2) {
            printf("%s wins!\n", ai);
            config->win = true;
            break;
        }

        if (game->current_player == PLAYER_ONE) {
            printf("%s turn:\n", me);
            Move move = terminal_read_move(game,
                                           undosLeft(game, &config->history));
            if (move.col == -1) return;          /* input closed */
            if (move.col == -2) {                /* 0 = save and quit */
                if (!saveGame(game, config)) continue;
                printf("Exiting...\n");
                return;
            }
            if (move.col == -4) {                /* u = undo */
                if (undoMove(game, &config->history, me, last,
                             sizeof last)) {
                    terminal_clear_screen();
                }
                continue;
            }
            if (!playMove(game, &config->history, move, me, last, sizeof last)) continue;
        } else {
            printf("%s turn:\n", ai);

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
                if (!playMove(game, &config->history, move, ai, last, sizeof last)) continue;
            } else {
                int timeBudget = (difficulty == AI_NORMAL) ? 100 : 1000;
                Move move = find_best_move(game, timeBudget);
                if (!playMove(game, &config->history, move, ai, last, sizeof last)) continue;
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
