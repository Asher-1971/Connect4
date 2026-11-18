#include <stdio.h>
#include "header.h"

// functions
void printBoard(int rows, int cols, char board[][cols]){
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

void initBoardColsFilled(int rows, int boardColsFilled[rows]){
    for (int i = 0; i < rows; i++) {
        boardColsFilled[i] = rows - 1; 
    }
}


int main(void){

    printf("choose what board size you want to play with\n");
    printf("1. Small 5x4\n");
    printf("2. Medium 6x5\n");
    printf("3. Classic 6x7\n");
    printf("4. Large 10x7\n");

    int rows, cols;

    int choice;
    printf("enter your choice: ");
    scanf("%d", &choice);

    // validate choice
    while (choice < 1 || choice > 4) {
        printf("invalid choice\n");
        printf("enter your choice: ");
        scanf("%d", &choice);
    }

    // process choice
    switch(choice) {
        case 1:
            // initialize 5x4 board
            rows = 5;
            cols = 4;
            break;
        case 2:
            // initialize 6x5 board
            rows = 6;
            cols = 5;
            break;
        case 3:
            // initialize 6x7 board
            rows = 6;
            cols = 7;
            break;
        case 4:
            // initialize 10x7 board
            rows = 10;
            cols = 7;
            break;
    }

    // initialize array(board)
    char board[rows][cols];

    int boardColsFilled[rows]; // how much each column is filled

    initBoardColsFilled(rows, boardColsFilled);
    initBoard(rows, cols, board);
    printBoard(rows, cols, board);

    int win = 0;
    int tie = 0;

    int colChoice;
    
    int turn = 1; // 1 for player 1, 2 for player 2
    
    // this is jst for me(the end variable)

    int end = 0;

    int AIorNot;
    printf("1. AI\n2. Human\nchoice: ");
    scanf("%d", &AIorNot);
    // a diff loop for AI ig?
    // GAME LOOOPP!!!!!!
    switch(AIorNot){
        case 1:
        do{
            printf("do u wanna end?");
            scanf("%d", &end);
            printf("player %d's turn\n", turn);
            printf("enter the column number to add a piece\n");
            scanf("%d", &colChoice);
            while (boardColsFilled[colChoice-1] < 0) {
                    printf("column is full, choose another column\n");
                    scanf("%d", &colChoice);
                }
            if (turn == 1) {
                board[boardColsFilled[colChoice-1]][colChoice-1] = P1;
                turn = 2;
                boardColsFilled[colChoice-1]--;
                
            } else {
                board[boardColsFilled[colChoice-1]][colChoice-1] = P2;
                turn = 1;
                boardColsFilled[colChoice-1]--;
            }
            printBoard(rows, cols, board);

        } while (end==0);
        break;
        case 2:
        // the second while loop
        break;
    }

    // AAAGGGHHHHHHHHHHHHHHHHHH

    return 0;
}