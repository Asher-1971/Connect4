#include <stdio.h>
#include "header.h"

// functions
void printBoard(int rows, int cols, char board[][cols]){
    for (int j = 0; j < cols; j++) {
        printf("%d ", j + 1);   
    }

    printf("\n");
    for (int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++){
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

void initBoard(int rows, int cols, char board[][cols]){
    for (int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++){
            board[i][j] = EMPTY;
        }
    }
}

void initBoardColsFilled(int rows,int cols, int boardColsFilled[]){
    for (int i = 0; i < cols; i++) {
        boardColsFilled[i] = rows - 1; 
    }
}

void dropPiece(int cols, char board[][cols], int boardColsFilled[], int colChoice, int *turn){
    int col = colChoice - 1;
    int row = boardColsFilled[col];

    if (*turn == 1) {
        board[row][col] = P1;
        *turn = 2;
    } else {
        board[row][col] = P2;
        *turn = 1;
    }

    boardColsFilled[col]--;
}

int readInt(void){
    int x;
    while (scanf("%d", &x) != 1) {
        while (getchar() != '\n');
        printf("enter a number: ");
    }
    return x;
}

int validateBoardSize(int n){
    return n >= MIN_SIZE && n <= MAX_SIZE;
}
int main(void){

    printf("choose what board size you want to play with\n");
    printf("1. Classic 6x7\n");
    printf("2. custom size\n");

    int rows, cols;

    int choice;
    printf("enter your choice: ");
    choice = readInt();

    // validate choice
    while (choice < 1 || choice > 2) {
        printf("invalid choice\n");
        printf("enter your choice: ");
        choice = readInt();
    }

    // process choice
    switch(choice) {
        case 1:
            // initialize 6x7 board
            rows = 6;
            cols = 7;
            break;
        case 2:
            // initialize custom size board
            printf("rows: ");
            rows = readInt();
            while (!validateBoardSize(rows)) {
                printf("invalid, enter rows (%d-%d): ", MIN_SIZE, MAX_SIZE);
                rows = readInt();
        }
            printf("columns: ");
            cols = readInt();
            while (!validateBoardSize(cols)) {
                printf("invalid, enter columns (%d-%d): ", MIN_SIZE, MAX_SIZE);
                cols = readInt();
            }
            break;
    }

    int AIorNot;
    printf("1. AI\n2. Human\nchoice: ");
    AIorNot = readInt();

    // initialize array(board)
    char board[rows][cols];

    int boardColsFilled[cols]; // how much each column is filled

    initBoardColsFilled(rows, cols, boardColsFilled);
    initBoard(rows, cols, board);
    printBoard(rows, cols, board);

    int win = 0;
    int tie = 0;
    int moves = 0;
    int colChoice;
    
    int turn = 1; // 1 for player 1, 2 for player 2
    

    // GAME LOOOPP!!!!!!
    switch(AIorNot){
        case 2:
            while (win == 0 && tie == 0) {
                printf("player %d's turn\n", turn);
                printf("enter the column number to add a piece\n");
                colChoice = readInt();
                while (colChoice < 1 || colChoice > cols || boardColsFilled[colChoice-1] < 0) {
                    printf("invalid or full column, choose another: ");
                    colChoice = readInt();
                }
                dropPiece(cols, board, boardColsFilled, colChoice, &turn);
                moves++;
                printBoard(rows, cols, board);
                if (checkWin(rows, cols, board, P1)) {
                    printf("player 1 wins!\n");
                    win = 1;
                } else if (checkWin(rows, cols, board, P2)) {
                    printf("player 2 wins!\n");
                    win = 1;
                } else if(checkTie(rows, cols, moves)) {
                    printf("it's a tie!\n");
                    tie = 1;
                }

            };
            break;
        case 1:
            // the second while loop
            break;
    }

    // AAAGGGHHHHHHHHHHHHHHHHHH

    return 0;
}