#ifndef HEADER_H
#define HEADER_H

#include <stdbool.h>
#include <stdint.h>

#define EMPTY 0
#define P1 1
#define P2 2
#define MIN_SIZE 4
#define MAX_SIZE 20
#define MAX_PLAYERS 2

#define PLAYER_NONE 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2

typedef enum {
    MOVE_NORMAL = 0,
    MOVE_BOMB = 1,
    MOVE_ANVIL = 2
} MoveType;

typedef struct {
    int col;
    MoveType type;
} Move;

typedef struct {
    int rows;
    int cols;
    int win_length;
    uint8_t board[MAX_SIZE][MAX_SIZE];
    int current_player;
    int move_count;
    int piece_count;
    bool bomb_used[MAX_PLAYERS + 1];
    bool anvil_used[MAX_PLAYERS + 1];
    bool powerups;   /* chosen in the menu: bomb + anvil allowed this game */
} GameState;

/* Player names: up to 15 characters plus the closing '\0'. */
#define NAME_LEN 16

/* Enough for any game: a full 20x20 board plus the discs power-ups can clear. */
#define MAX_MOVES 512

/* Every move made so far, kept outside GameState so the AI's copies stay
 * small. Undo replays this list from an empty board without the last moves. */
typedef struct {
    int count;
    Move moves[MAX_MOVES];
    int undos_used[MAX_PLAYERS + 1];
} MoveHistory;

#endif
