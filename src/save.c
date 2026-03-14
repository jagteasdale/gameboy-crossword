#include <gb/gb.h>
#include <string.h>
#include "save.h"

// SRAM starts at 0xA000
#define SRAM_START ((uint8_t*)0xA000)

// Get pointer to save slot for a puzzle
static SaveData* get_save_slot(uint8_t puzzle_index) {
    if (puzzle_index >= MAX_SAVED_PUZZLES) return NULL;
    return (SaveData*)(SRAM_START + (puzzle_index * sizeof(SaveData)));
}

void save_init(void) {
    // Nothing to initialize - SRAM access handled per-operation
}

void save_puzzle(uint8_t puzzle_index, const Puzzle* puzzle,
                 uint8_t clue_index, uint8_t word_offset, uint8_t direction) {
    SaveData* slot = get_save_slot(puzzle_index);
    if (slot == NULL) return;

    ENABLE_RAM;

    slot->magic = SAVE_MAGIC;
    slot->puzzle_index = puzzle_index;
    slot->clue_index = clue_index;
    slot->word_offset = word_offset;
    slot->direction = direction;
    // Don't overwrite completed flag if already set
    // slot->completed stays as-is

    // Copy player inputs from puzzle grid
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            slot->inputs[y][x] = puzzle->grid[y][x].player_input;
        }
    }

    DISABLE_RAM;
}

uint8_t save_load(uint8_t puzzle_index, Puzzle* puzzle,
                  uint8_t* clue_index, uint8_t* word_offset, uint8_t* direction) {
    SaveData* slot = get_save_slot(puzzle_index);
    if (slot == NULL) return 0;

    ENABLE_RAM;

    // Check for valid save
    if (slot->magic != SAVE_MAGIC || slot->puzzle_index != puzzle_index) {
        DISABLE_RAM;
        return 0;
    }

    // Restore position state
    *clue_index = slot->clue_index;
    *word_offset = slot->word_offset;
    *direction = slot->direction;

    // Restore player inputs to puzzle grid
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            // Only restore to non-black cells
            if (puzzle->grid[y][x].solution != CELL_BLACK) {
                puzzle->grid[y][x].player_input = slot->inputs[y][x];
            }
        }
    }

    DISABLE_RAM;
    return 1;
}

uint8_t save_exists(uint8_t puzzle_index) {
    SaveData* slot = get_save_slot(puzzle_index);
    if (slot == NULL) return 0;

    ENABLE_RAM;
    uint8_t exists = (slot->magic == SAVE_MAGIC && slot->puzzle_index == puzzle_index);
    DISABLE_RAM;

    return exists;
}

void save_mark_complete(uint8_t puzzle_index) {
    SaveData* slot = get_save_slot(puzzle_index);
    if (slot == NULL) return;

    ENABLE_RAM;
    slot->magic = SAVE_MAGIC;
    slot->puzzle_index = puzzle_index;
    slot->completed = 1;
    DISABLE_RAM;
}

uint8_t save_is_complete(uint8_t puzzle_index) {
    SaveData* slot = get_save_slot(puzzle_index);
    if (slot == NULL) return 0;

    ENABLE_RAM;
    uint8_t complete = (slot->magic == SAVE_MAGIC &&
                        slot->puzzle_index == puzzle_index &&
                        slot->completed == 1);
    DISABLE_RAM;

    return complete;
}

void save_clear(uint8_t puzzle_index) {
    SaveData* slot = get_save_slot(puzzle_index);
    if (slot == NULL) return;

    ENABLE_RAM;
    slot->magic = 0;  // Invalidate save
    slot->completed = 0;
    DISABLE_RAM;
}
