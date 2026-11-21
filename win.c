#include "header.h"

int countInLine(GameState *game, int r, int c, int dr, int dc, char piece) {
    if (r < 0 || r >= game->rows || c < 0 || c >= game->cols) {
        return 0;
    }
    if (game->board[r][c] != piece) {
        return 0;
    }
    return 1 + countInLine(game, r + dr, c + dc, dr, dc, piece);
}

int checkWin(GameState *game, char piece) {
    int target = (game->winLength >= 4) ? game->winLength : 4;
    for (int r = 0; r < game->rows; r++) {
        for (int c = 0; c < game->cols; c++) {
            if (countInLine(game, r, c, 0, 1, piece) >= target) return 1;   /* Right */
            if (countInLine(game, r, c, 1, 0, piece) >= target) return 1;   /* Down */
            if (countInLine(game, r, c, 1, 1, piece) >= target) return 1;   /* Diagonal down-right */
            if (countInLine(game, r, c, 1, -1, piece) >= target) return 1;  /* Diagonal down-left */
        }
    }
    return 0;
}

int checkTie(GameState *game) {
    return game->moves >= game->rows * game->cols;
}


/* ============================================================================
 * FILE 5: start.c
 * ============================================================================ */
#include <stdio.h>
#include "header.h"
#include "module3.h"

void printBoard(GameState *game) {
    for (int j = 0; j < game->cols; j++) {
        printf("%d ", j + 1);
    }

    printf("\n");
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            printf("%c ", game->board[i][j]);
        }
        printf("\n");
    }
}

void initBoard(GameState *game) {
    for (int i = 0; i < game->rows; i++) {
        for (int j = 0; j < game->cols; j++) {
            game->board[i][j] = EMPTY;
        }
    }
}

void initBoardColsFilled(GameState *game) {
    for (int i = 0; i < game->cols; i++) {
        game->boardColsFilled[i] = game->rows - 1;
    }
}

/* Returns 1 on successful drop, 0 if game/column/row/turn is invalid or column is full */
int dropPiece(GameState *game) {
    if (!game) {
        return 0;
    }
    if (game->rows <= 0 || game->rows > MAX_SIZE || game->cols <= 0 || game->cols > MAX_SIZE) {
        return 0;
    }

    int col = game->colChoice - 1;
    if (col < 0 || col >= game->cols) {
        return 0;
    }

    int row = game->boardColsFilled[col];
    if (row < 0 || row >= game->rows) {
        return 0;
    }

    if (game->turn == 1) {
        game->board[row][col] = P1;
        game->turn = 2;
    } else if (game->turn == 2) {
        game->board[row][col] = P2;
        game->turn = 1;
    } else {
        return 0;
    }

    game->boardColsFilled[col]--;
    return 1;
}

int readInt(void) {
    int x;
    while (scanf("%d", &x) != 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        if (ch == EOF) {
            return -1;
        }
        printf("enter a number: ");
    }
    return x;
}

int validateBoardSize(int n) {
    return n >= MIN_SIZE && n <= MAX_SIZE;
}

int mainstart(void) {
    GameState game = {0};

    printf("choose what board size you want to play with\n");
    printf("1. Classic 6x7\n");
    printf("2. custom size\n");

    printf("enter your choice: ");
    game.choice = readInt();
    if (game.choice == -1) return 0;

    while (game.choice < 1 || game.choice > 2) {
        printf("invalid choice\n");
        printf("enter your choice: ");
        game.choice = readInt();
        if (game.choice == -1) return 0;
    }

    switch (game.choice) {
        case 1:
            game.rows = 6;
            game.cols = 7;
            break;
        case 2:
            printf("rows: ");
            game.rows = readInt();
            if (game.rows == -1) return 0;
            while (!validateBoardSize(game.rows)) {
                printf("invalid, enter rows (%d-%d): ", MIN_SIZE, MAX_SIZE);
                game.rows = readInt();
                if (game.rows == -1) return 0;
            }
            printf("columns: ");
            game.cols = readInt();
            if (game.cols == -1) return 0;
            while (!validateBoardSize(game.cols)) {
                printf("invalid, enter columns (%d-%d): ", MIN_SIZE, MAX_SIZE);
                game.cols = readInt();
                if (game.cols == -1) return 0;
            }
            break;
    }

    printf("choose win condition (4 for Connect-4, 5 for Connect-5): ");
    game.winLength = readInt();
    if (game.winLength == -1) return 0;
    while (game.winLength != 4 && game.winLength != 5) {
        printf("invalid choice, enter 4 or 5: ");
        game.winLength = readInt();
        if (game.winLength == -1) return 0;
    }
    if (game.winLength == 5 && (game.rows < 5 || game.cols < 5)) {
        printf("board too small for Connect-5, defaulting to Connect-4.\n");
        game.winLength = 4;
    }

    printf("1. AI\n2. Human\nchoice: ");
    game.aiOrNot = readInt();
    if (game.aiOrNot == -1) return 0;
    while (game.aiOrNot < 1 || game.aiOrNot > 2) {
        printf("invalid choice\n");
        printf("1. AI\n2. Human\nchoice: ");
        game.aiOrNot = readInt();
        if (game.aiOrNot == -1) return 0;
    }

    initBoardColsFilled(&game);
    initBoard(&game);
    printBoard(&game);

    game.turn = 1;

    switch (game.aiOrNot) {
        case 1:
            while (game.win == 0 && game.tie == 0) {
                if (game.turn == 1) {
                    printf("player 1's turn\n");
                    printf("enter the column number to add a piece: ");
                    game.colChoice = readInt();
                    if (game.colChoice == -1) return 0;
                    while (game.colChoice < 1 || game.colChoice > game.cols ||
                           game.boardColsFilled[game.colChoice - 1] < 0) {
                        printf("invalid or full column, choose another: ");
                        game.colChoice = readInt();
                        if (game.colChoice == -1) return 0;
                    }
                } else {
                    game.colChoice = getBestMove(&game, game.winLength, 400);
                    if (game.colChoice < 1 || game.colChoice > game.cols) {
                        printf("no valid AI move available, it's a tie!\n");
                        game.tie = 1;
                        break;
                    }
                    printf("AI chose column %d\n", game.colChoice);
                }

                if (!dropPiece(&game)) {
                    printf("failed to place piece in column %d.\n", game.colChoice);
                    break;
                }
                game.moves++;
                printBoard(&game);

                if (checkWin(&game, P1)) {
                    printf("player 1 wins!\n");
                    game.win = 1;
                } else if (checkWin(&game, P2)) {
                    printf("AI wins!\n");
                    game.win = 1;
                } else if (checkTie(&game)) {
                    printf("it's a tie!\n");
                    game.tie = 1;
                }
            }
            break;

        case 2:
            while (game.win == 0 && game.tie == 0) {
                printf("player %d's turn\n", game.turn);
                printf("enter the column number to add a piece\n");
                game.colChoice = readInt();
                if (game.colChoice == -1) return 0;
                while (game.colChoice < 1 || game.colChoice > game.cols ||
                       game.boardColsFilled[game.colChoice - 1] < 0) {
                    printf("invalid or full column, choose another: ");
                    game.colChoice = readInt();
                    if (game.colChoice == -1) return 0;
                }
                if (!dropPiece(&game)) {
                    printf("failed to place piece.\n");
                    break;
                }
                game.moves++;
                printBoard(&game);
                if (checkWin(&game, P1)) {
                    printf("player 1 wins!\n");
                    game.win = 1;
                } else if (checkWin(&game, P2)) {
                    printf("player 2 wins!\n");
                    game.win = 1;
                } else if (checkTie(&game)) {
                    printf("it's a tie!\n");
                    game.tie = 1;
                }
            }
            break;
    }

    return 0;
}

/* Remove this main() wrapper ONLY if another file (like main.c) defines main() */
int main(void) {
    return mainstart();
}