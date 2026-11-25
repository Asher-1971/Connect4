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

#endif
