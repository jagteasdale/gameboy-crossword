#ifndef PUZZLES_H
#define PUZZLES_H

#include "crossword_types.h"

// Number of puzzles available
#define PUZZLE_COUNT 1

// Get a puzzle by index
const Puzzle* puzzles_get(uint8_t index);

// Get puzzle title for menu display
const char* puzzles_get_title(uint8_t index);

// Initialize puzzle player grid (copy solution structure, clear inputs)
void puzzles_init_player_grid(Puzzle* dest, uint8_t puzzle_index);

#endif // PUZZLES_H
