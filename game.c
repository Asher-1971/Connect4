#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "module3.h"

void wait_for_enter(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void clearScreen(void)
{
    printf("\033[3J\033[2J\033[H");
    fflush(stdout);
}

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

void gameInputs(GameState *game)
{
    if (!game) {
        return;
    }

    printf("Connect 4 Game\n");
    printf("1. New Game\n");
    printf("2. Resume Game\n");
    printf("Enter your choice: ");

    game->choice = readInt();
    while (game->choice < 1 || game->choice > 2) {
        if (game->choice == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        game->choice = readInt();
    }

    if (game->choice == 2) {
        if (!loadGame(game)) {
            game->choice = 0;
        }
        return;
    }

    printf("\nBoard size:\n");
    printf("1. Classic 6x7\n");
    printf("2. Custom size\n");
    printf("Enter your choice: ");

    int sizeChoice = readInt();
    while (sizeChoice < 1 || sizeChoice > 2) {
        if (sizeChoice == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        sizeChoice = readInt();
    }

    if (sizeChoice == 1) {
        game->rows = 6;
        game->cols = 7;
    } else {
        printf("Rows (%d-%d): ", MIN_SIZE, MAX_SIZE);
        game->rows = readInt();
        while (!validateBoardSize(game->rows)) {
            if (game->rows == -1) {
                return;
            }
            printf("Invalid rows. Enter %d-%d: ", MIN_SIZE, MAX_SIZE);
            game->rows = readInt();
        }

        printf("Columns (%d-%d): ", MIN_SIZE, MAX_SIZE);
        game->cols = readInt();
        while (!validateBoardSize(game->cols)) {
            if (game->cols == -1) {
                return;
            }
            printf("Invalid columns. Enter %d-%d: ", MIN_SIZE, MAX_SIZE);
            game->cols = readInt();
        }
    }

    printf("Win length (4 or 5): ");
    game->winLength = readInt();
    while (game->winLength < 4 || game->winLength > 5 ||
           (game->winLength > game->rows && game->winLength > game->cols)) {
        if (game->winLength == -1) {
            return;
        }
        printf("Invalid win length. Enter 4 or 5: ");
        game->winLength = readInt();
    }

    printf("\nChoose game mode:\n");
    printf("1. Human vs Human\n");
    printf("2. Human vs AI\n");
    printf("Enter your choice: ");

    game->aiOrNot = readInt();
    while (game->aiOrNot < 1 || game->aiOrNot > 2) {
        if (game->aiOrNot == -1) {
            return;
        }
        printf("Enter 1 or 2: ");
        game->aiOrNot = readInt();
    }

    if (game->aiOrNot == 2) {
        printf("\nChoose AI difficulty:\n");
        printf("1. Easy\n");
        printf("2. Normal\n");
        printf("3. Hard\n");
        printf("Enter difficulty: ");

        game->choice = readInt();
        while (game->choice < AI_EASY || game->choice > AI_HARD) {
            if (game->choice == -1) {
                return;
            }
            printf("Enter 1, 2, or 3: ");
            game->choice = readInt();
        }
    }

    game->win = 0;
    game->tie = 0;
    game->move_count = 0;
    game->piece_count = 0;
    game->current_player = PLAYER_ONE;
    game->colChoice = 0;
    initBoard(game);
    initBoardColsFilled(game);
}

static int promptColumn(GameState *game)
{
    if (!game) {
        return -1;
    }

    printf("Enter a column (1-%d), or 0 to save and quit: ", game->cols);
    int column = readInt();
    if (column == -1) {
        return -1;
    }

    while (column < 0 || column > game->cols ||
           (column > 0 && game->boardColsFilled[column - 1] < 0)) {
        printf("Invalid or full column. Enter 0-%d: ", game->cols);
        column = readInt();
        if (column == -1) {
            return -1;
        }
    }

    return column;
}

void uiHumanvsHuman(GameState *game)
{
    if (!game) {
        return;
    }

    while (game->win == 0 && game->tie == 0) {
        printBoard(game);

        if (checkWin(game, P1) || checkWin(game, P2)) {
            char winner = checkWin(game, P1) ? P1 : P2;
            printf("Player %c wins!\n", winner);
            game->win = 1;
            break;
        }
        if (checkTie(game)) {
            printf("It's a draw!\n");
            game->tie = 1;
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

        game->colChoice = column;
        if (!dropPiece(game)) {
            continue;
        }

        game->moves++;
        clearScreen();
    }
}

void uiHumanvsAI(GameState *game)
{
    if (!game) {
        return;
    }

    int difficulty = game->choice;
    if (difficulty < AI_EASY || difficulty > AI_HARD) {
        difficulty = AI_NORMAL;
    }

    if (difficulty == AI_EASY) {
        srand((unsigned int)time(NULL));
    }

    while (game->win == 0 && game->tie == 0) {
        printBoard(game);

        if (checkWin(game, P1)) {
            printf("You win!\n");
            game->win = 1;
            break;
        }
        if (checkWin(game, P2)) {
            printf("AI wins!\n");
            game->win = 1;
            break;
        }
        if (checkTie(game)) {
            printf("It's a draw!\n");
            game->tie = 1;
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
            game->colChoice = column;
        } else {
            printf("AI turn (%c):\n", P2);

            if (difficulty == AI_EASY) {
                int valid[20];
                int validCount = 0;
                for (int c = 0; c < game->cols; c++) {
                    if (game->boardColsFilled[c] < game->rows) {
                        valid[validCount++] = c + 1;
                    }
                }
                game->colChoice = validCount > 0
                    ? valid[rand() % validCount]
                    : -1;
            } else {
                int timeBudget = (difficulty == AI_NORMAL) ? 100 : 1000;
                game->colChoice = getBestMove(
                    game, game->winLength, timeBudget);
            }
        }

        if (game->colChoice < 1 || game->colChoice > game->cols ||
            game->boardColsFilled[game->colChoice - 1] < 0) {
            continue;
        }

        if (!dropPiece(game)) {
            continue;
        }

        game->moves++;
        clearScreen();
    }
}

int main(void)
{
    GameState game = {0};
    gameInputs(&game);

    if (game.choice == 0) {
        return 0;
    }

    if (game.aiOrNot == 1) {
        uiHumanvsHuman(&game);
    } else if (game.aiOrNot == 2) {
        uiHumanvsAI(&game);
    }

    return 0;
}
