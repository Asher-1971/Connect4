#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include "header.h"
#include "module3.h"

#define MAX_ROWS 20
#define MAX_COLS 20
#define DEFAULT_ROWS 6
#define DEFAULT_COLS 7

void wait_for_enter(void)
{
    int c;
    /* Clear any leftover characters up to the current newline */
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void clearScreen(void)
{
    printf("Press Enter to continue...");
    fflush(stdout);

    wait_for_enter();

    /* Clear visible screen + scrollback buffer, reset cursor */
    printf("\033[3J\033[2J\033[H");
    fflush(stdout);
}

void gameInputs(GameState *game)
{
  int *gameChoice;
  int *gameMode;
  int aiDifficulty;

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
  int contChoice;
  int gameOver=0;

  while(!gameOver)
  {
    printBoard(GameState *game);

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

    //checking if user wants to continue or not
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
      //input function mainstart()
    }

    clearScreen();
    
  }

}

//uifunction2
void uiHumanvsAI(GameState *game)
{
  char humanPiece='X';
  char aiPiece='O';
  char currentPiece=humanPiece;
  int contChoice;
  int gameOver=0;

  while(!gameOver)
  {
    printBoard(GameState *game);

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
      //input function
    }

    clearScreen();
}
}

int main()
{
  
}