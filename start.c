#include <stdio.h>
#include "header.h"

// functions
void printBoard(GameState *game){
    for (int j = 0; j < game->cols; j++) {
        printf("%d ", j + 1);   
    }

    printf("\n");
    for (int i = 0; i < game->rows; i++) {
        for(int j = 0; j < game->cols; j++){
            printf("%c ", game->board[i][j]);
        }
        printf("\n");
    }
}

void initBoard(GameState *game){
    for (int i = 0; i < game->rows; i++) {
        for(int j = 0; j < game->cols; j++){
            game->board[i][j] = EMPTY;
        }
    }
}

void initBoardColsFilled(GameState *game){
    for (int i = 0; i < game->cols; i++) {
        game->boardColsFilled[i] = game->rows - 1;
    }
}

void dropPiece(GameState *game){
    int col = game->colChoice - 1;
    int row = game->boardColsFilled[col];

    if (game->turn == 1) {
        game->board[row][col] = P1;
        game->turn = 2;
    } else {
        game->board[row][col] = P2;
        game->turn = 1;
    }

    game->boardColsFilled[col]--;
}

int readInt(void){
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

int validateBoardSize(int n){
    return n >= MIN_SIZE && n <= MAX_SIZE;
}
int mainstart(void){
    GameState game = {0};

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
            printf("AI mode is not implemented yet.\n");
            break;
    }

    // AAAGGGHHHHHHHHHHHHHHHHHH

    return 0;
}