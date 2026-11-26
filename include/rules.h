#ifndef RULES_H
#define RULES_H

#include <stdbool.h>
#include "header.h"

bool rules_validate_state(const GameState *state);
bool rules_is_valid_move(const GameState *state, Move move);
bool rules_apply_move(const GameState *state, Move move, GameState *out);
void rules_apply_gravity(GameState *state);
int rules_check_winner(const GameState *state);
bool rules_is_draw(const GameState *state);
bool rules_is_terminal(const GameState *state);
bool rules_replay(const GameState *settings, const Move *moves, int count,
                  GameState *out);
int rules_undo_limit(const GameState *state);

#endif