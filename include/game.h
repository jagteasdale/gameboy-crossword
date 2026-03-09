#ifndef GAME_H
#define GAME_H

#include "crossword_types.h"

// Initialize the game
void game_init(void);

// Main game loop tick
void game_update(void);

// Load a puzzle into memory
void game_load_puzzle(uint8_t puzzle_index);

// Check if the puzzle is complete
uint8_t game_check_complete(void);

// Get current game state
GameState game_get_state(void);

// Set game state
void game_set_state(GameState state);

#endif // GAME_H
