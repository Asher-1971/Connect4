#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include "header.h"

#define MAX_ROWS 20
#define MAX_COLS 20
#define DEFAULT_ROWS 6
#define DEFAULT_COLS 7

void clearScreen()
{
  printf("\033[3J\033[2J\033[H");
  fflush(stdout);
}

void gameInputs(int *gameChoice, int *gameMode, int *aiDifficulty, int *rows, int *cols)
{

printf("Connect 4 Game\n");
printf("1.New Game\n");
printf("2.Resume Game\n");
printf("Enter your choice: ");
if(scanf("%d", gameChoice)!=1)
{
  *gameChoice=0;
  return;
}

if(*gameChoice==1)
{
  //ameera's size function

  printf("Choose game mode\n");
  printf("1.Human vs Human\n");
  printf("2.Human vs AI\n");
  scanf("%d", gameMode);

  if(*gameMode==2)
  {
    printf("Choose AI Difficulty:\n");
    printf("1.Easy\n");
    printf("2.Normal\n");
    printf("Enter Difficulty: ");
    scanf("%d", aiDifficulty);
  }
}
else if(*gameChoice==2)
{
   loadGame();
}

}

//uiFunction1
void uiHumanvsHuman(GameState *game)
{
  char p1 = 'X';
  char p2 = 'O';
  char currentPiece=p1;
  char contChoice[16];
  int gameOver=0;

  while(!gameOver)
  {
    //ameera printBoard function

    //input function
    printf("player %c turn:\n",currentPiece);

    //ameera win/draw check
    if(checkWin(GameState *game)==1)
    {
      //printboard(rows,cols,board)
      printf("Player %c wins!\n",currentPiece);
      gameOver=1;
      break;
    }
    if(checkTie(GameState *game)==1)
    {
      //printboard(rows,cols,board)
      printf("Its a draw!\n");
      gameOver=1;
      break;
    }

    printf("Enter column number or 0 to save and quit the game:\n");
    scanf("%d", contChoice);
    if(contChoice==0)
    {
      //call savegame() function
      printf("Game saved! Exiting...\n");
      gameOver=1;
    }
    if(contChoice!=0)
    {
      
    }

    //checking if user wants to continue or not
    printf("Enter 'exit' to save and quit, or anything else to continue: ");
    if(scanf("%15s", contChoice)!=1)
    {
      break;
    }

    if(strcmp(contChoice, "exit")==0 || strcmp(contChoice, "EXIT")==0)
    {
      //call savegame() function
      printf("Game saved! Exiting...\n");
      gameOver=1;
    }
    else
    {
      currentPiece = (currentPiece==p1) ? p2 : p1;
      clearScreen();
    }
  }

}

//uifunction2
void uiHumanvsAI(GameState *game)
{
  char humanPiece='X';
  char aiPiece='O';
  char currentPiece=humanPiece;
  char contChoice[16];
  int gameOver=0;

  while(!gameOver)
  {
    //ameera printBoard function

    //input function
    if(currentPiece==humanPiece)
    {
      printf("Your Turn (%c):\n",humanPiece);
    }
    else
    {
      printf("AI turn (%c):\n",aiPiece);
    }

    //ameera win/draw check
    if(checkWin(GameState *game)==1)
    {
      //printboard(rows,cols,board)
      if(currentPiece==humanPiece)
      {
        printf("You win!\n");
      }
      else
      {
        printf("AI wins!\n");
      }
      gameOver=1;
      break;
    }
    if(checkDraw(GameState *game)==1)
    {
      //printboard(rows,cols,board)
      printf("Its a draw!\n");
      gameOver=1;
      break;
    }

    //checking if user wants to continue or not
    printf("Enter 'exit' to save and quit, or anything else to continue: ");
    if(scanf("%15s", contChoice)!=1)
    {
      break;
    }

    if(strcmp(contChoice, "exit")==0 || strcmp(contChoice, "EXIT")==0)
    {
      //call savegame() function
      printf("Game saved! Exiting...\n");
      gameOver=1;
    }
    else
    {
      currentPiece = (currentPiece==humanPiece) ? aiPiece : humanPiece;
      clearScreen();
    }
  }

}

int main(void)
{
    int gameChoice = 0;
    int gameMode = 0;
    int aiDifficulty = 1;
    int rows = DEFAULT_ROWS;
    int cols = DEFAULT_COLS;

    printf("\033[?1049h\033[3J\033[2J\033[H");
    fflush(stdout);

    gameInputs(&gameChoice, &gameMode, &aiDifficulty, &rows, &cols);

    char board[rows][cols];
    memset(board, ' ', sizeof(board));

    if (gameMode == 1)
    {
        uiHumanVsHuman(rows, cols, board);
    }
    else if (gameMode == 2)
    {
        uiHumanVsAI(rows, cols, board, aiDifficulty);
    }

    printf("\033[?1049l");
    fflush(stdout);

    return 0;
}