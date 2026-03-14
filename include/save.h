#ifndef SAVE_H
#define SAVE_H

#include <gb/gb.h>
#include "crossword_types.h"

// Save data structure for one puzzle (232 bytes)
typedef struct {
    uint8_t magic;              // Magic number to detect valid save
    uint8_t puzzle_index;       // Which puzzle this save is for
    uint8_t clue_index;         // Current clue index
    uint8_t word_offset;        // Position within current word
    uint8_t direction;          // DIR_ACROSS or DIR_DOWN
    uint8_t completed;          // 1 if puzzle completed
    uint8_t padding[2];         // Alignment padding
    uint8_t inputs[GRID_HEIGHT][GRID_WIDTH];  // Player inputs (225 bytes)
} SaveData;

#define SAVE_MAGIC 0x43  // 'C' for crossword

// Maximum puzzles we can save (8KB SRAM / 232 bytes per save)
#define MAX_SAVED_PUZZLES 32

// Initialize save system
void save_init(void);

// Save current puzzle progress
void save_puzzle(uint8_t puzzle_index, const Puzzle* puzzle,
                 uint8_t clue_index, uint8_t word_offset, uint8_t direction);

// Load saved puzzle progress (returns 1 if save found, 0 if not)
uint8_t save_load(uint8_t puzzle_index, Puzzle* puzzle,
                  uint8_t* clue_index, uint8_t* word_offset, uint8_t* direction);

// Check if a puzzle has a save
uint8_t save_exists(uint8_t puzzle_index);

// Mark puzzle as completed
void save_mark_complete(uint8_t puzzle_index);

// Check if puzzle is completed
uint8_t save_is_complete(uint8_t puzzle_index);

// Clear save for a puzzle
void save_clear(uint8_t puzzle_index);

#endif // SAVE_H
