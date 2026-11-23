#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "header.h"
#include "module3.h"

// functions
void printBoard(GameState *game){
    if (!game) return;
    for (int j = 0; j < game->cols; j++) {
        printf("%d ", j + 1);   
    }

    printf("\n");
    for (int i = game->rows - 1; i >= 0; i--) {
        for(int j = 0; j < game->cols; j++){
            char piece = game->board[i][j] == P1 ? 'X' :
                         game->board[i][j] == P2 ? 'O' : '.';
            printf("%c ", piece);
        }
        printf("\n");
    }
}

void initBoard(GameState *game){
    if (!game) return;
    for (int i = 0; i < game->rows; i++) {
        for(int j = 0; j < game->cols; j++){
            game->board[i][j] = EMPTY;
        }
    }
    game->piece_count = 0;
    game->move_count = 0;
    game->current_player = PLAYER_ONE;
}

void initBoardColsFilled(GameState *game){
    if (!game) return;
    for (int i = 0; i < game->cols; i++) {
        game->boardColsFilled[i] = 0;
    }
}

int dropPiece(GameState *game){
    if (!game || game->colChoice < 1 || game->colChoice > game->cols) {
        return 0;
    }

    int col = game->colChoice - 1;
    int row = game->boardColsFilled[col];
    if (row < 0 || row >= game->rows ||
        game->current_player < PLAYER_ONE ||
        game->current_player > PLAYER_TWO) {
        return 0;
    }
    if (game->board[row][col] != EMPTY) {
        return 0;
    }

    game->board[row][col] = (uint8_t)game->current_player;
    game->boardColsFilled[col]++;
    game->piece_count++;
    game->move_count++;
    game->current_player =
        (game->current_player == PLAYER_ONE) ? PLAYER_TWO : PLAYER_ONE;
    return 1;
}

int readInt(void){
    char input[128];

    while (1) {
        if (!fgets(input, sizeof input, stdin)) {
            return -1;
        }

        if (!strchr(input, '\n')) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            if (ch == EOF) {
                return -1;
            }
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

        while (*end == ' ' || *end == '\t') {
            end++;
        }
        if (*end != '\n' && *end != '\r' && *end != '\0') {
            printf("enter a valid number: ");
            continue;
        }

        return (int)value;
    }
}

int validateBoardSize(int n){
    return n >= MIN_SIZE && n <= MAX_SIZE;
}
int mainstart(void){
    GameState game = {0};
    game.winLength = 4;

    printf("choose what board size you want to play with\n");
    printf("1. Classic 6x7\n");
    printf("2. custom size\n");

    printf("enter your choice: ");
    game.choice = readInt();
    if (game.choice == -1) return 0;

    // validate choice
    while (game.choice < 1 || game.choice > 2) {
        printf("invalid choice\n");
        printf("enter your choice: ");
        game.choice = readInt();
        if (game.choice == -1) return 0;
    }

    // process choice
    switch(game.choice) {
        case 1:
            // initialize 6x7 board
            game.rows = 6;
            game.cols = 7;
            break;
        case 2:
            // initialize custom size board
            printf("rows: ");
            game.rows = readInt();
            while (!validateBoardSize(game.rows)) {
                printf("invalid, enter rows (%d-%d): ", MIN_SIZE, MAX_SIZE);
                game.rows = readInt();
                if (game.rows == -1) return 0;
        }
            printf("columns: ");
            game.cols = readInt();
            while (!validateBoardSize(game.cols)) {
                printf("invalid, enter columns (%d-%d): ", MIN_SIZE, MAX_SIZE);
                game.cols = readInt();
                if (game.cols == -1) return 0;
            }
            break;
    }

    printf("win length (4-Connect 4, 5-Connect 5): ");
    game.winLength = readInt();
    if (game.winLength == -1) return 0;
    while (game.winLength < 4 || game.winLength > 5 ||
           (game.winLength > game.rows && game.winLength > game.cols)) {
        printf("invalid win length, enter 4 or 5: ");
        game.winLength = readInt();
        if (game.winLength == -1) return 0;
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

    game.turn = 1; // 1 for player 1, 2 for player 2

    // GAME LOOOPP!!!!!!
    switch(game.aiOrNot){
        case 2:
            while (game.win == 0 && game.tie == 0) {
                printf("player %d's turn\n", game.turn);
                printf("enter the column number to add a piece\n");
                game.colChoice = readInt();
                if (game.colChoice == -1) return 0;
                while (game.colChoice < 1 || game.colChoice > game.cols || game.boardColsFilled[game.colChoice-1] < 0) {
                    printf("invalid or full column, choose another: ");
                    game.colChoice = readInt();
                    if (game.colChoice == -1) return 0;
                }
                dropPiece(&game);
                game.moves++;
                printBoard(&game);
                if (checkWin(&game, P1)) {
                    printf("player 1 wins!\n");
                    game.win = 1;
                } else if (checkWin(&game, P2)) {
                    printf("player 2 wins!\n");
                    game.win = 1;
                } else if(checkTie(&game)) {
                    printf("it's a tie!\n");
                    game.tie = 1;
                }

            }
            break;
        case 1:
            while (game.win == 0 && game.tie == 0) {
                if (game.turn == 1) {
                    printf("your turn\n");
                    printf("enter the column number to add a piece\n");
                    game.colChoice = readInt();
                    if (game.colChoice == -1) return 0;
                    while (game.colChoice < 1 || game.colChoice > game.cols ||
                           game.boardColsFilled[game.colChoice - 1] < 0) {
                        printf("invalid or full column, choose another: ");
                        game.colChoice = readInt();
                        if (game.colChoice == -1) return 0;
                    }
                } else {
                    printf("AI is choosing a column...\n");
                    game.colChoice = getBestMove(&game, game.winLength, 100);
                    if (game.colChoice < 1 || game.colChoice > game.cols ||
                        game.boardColsFilled[game.colChoice - 1] < 0) {
                        break;
                    }
                }

                if (!dropPiece(&game)) {
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
    }

    // AAAGGGHHHHHHHHHHHHHHHHHH

    return 0;
}
