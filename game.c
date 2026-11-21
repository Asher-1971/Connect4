#include "header.h"

int **board=NULL;
int rows=6, cols=7, winLen=4;
int mode=TWO_PLAYER;
int level=EASY;
int current=1;
int bombs[3], anvils[3];
char playerName[3][NAME_LEN+1];
char symbol[3];

static char message[120]="";

static void clearScreen(void)
{
  #ifdef _WIN32
      system("cls");
  #else
      system("clear");
  #endif
}

static void readLine(char *buf, int size)
{
  int len, ch;
  if(fgets(buf, size, stdin)==NULL){
    printf("\n");
    exit(0);
  }
  len = (int)strlen(buf);
  if(len>0 && buf[len-1] == '\n') buf[len-1]='\0';
  else{
    while((ch=getchar()) != '\n' && ch!=EOF);
  }
}

static int readInt(const char *prompt, int low, int high)
{
  char buf[50];
  int v;

  while(1){
    printf("%s",prompt);
    readLine(buf, sizeof buf);
    v=atoi(buf);
    if(v>=low && v<=high) return v;
    printf("Please enter a number from %d to %d",low,high);
  }
}



