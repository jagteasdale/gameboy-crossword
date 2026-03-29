#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "crossword_types.h"

// Re-export tile indices for convenience
#define TILE_EMPTY       0x00
#define TILE_BLACK       0x01
#define TILE_CURSOR      0x02
#define TILE_ARROW_RIGHT 0x03

#define TILE_LETTER_A    0x10
#define TILE_NUMBER_0    0x30
#define TILE_COLON       0x3A
#define TILE_SPACE       0x3B
#define TILE_PERIOD      0x3C
#define TILE_HYPHEN      0x40

// Numbered empty cells (clue numbers 1-15)
#define TILE_CELL_NUM_1  0x44
#define TILE_CELL_NUM_15 0x52

// Initialize graphics subsystem
void graphics_init(void);

// Load tile data into VRAM
void graphics_load_tiles(void);

// Draw the crossword grid (visible portion)
void graphics_draw_grid(const Puzzle* puzzle, const ViewOffset* offset);

// Draw the cursor
void graphics_draw_cursor(const Cursor* cursor, const ViewOffset* offset);

// Clear cursor from previous position
void graphics_clear_cursor(uint8_t x, uint8_t y, const ViewOffset* offset);

// Draw a clue in the text area
void graphics_draw_clue(const Clue* clue, Direction dir);

// Draw the title screen
void graphics_draw_title(void);

// Draw the puzzle selection menu
void graphics_draw_menu(uint8_t selected_index, uint8_t puzzle_count);

// Draw completion screen
void graphics_draw_complete(void);

// Pause menu options
#define PAUSE_OPTION_RESUME 0
#define PAUSE_OPTION_QUIT   1
#define PAUSE_OPTION_COUNT  2

// Draw pause menu overlay with selected option
void graphics_draw_pause_menu(uint8_t selected_option);

// Draw a single cell
void graphics_draw_cell(uint8_t screen_x, uint8_t screen_y, const Cell* cell, uint8_t is_cursor);

// Draw text at position (for clues/messages)
void graphics_draw_text(uint8_t x, uint8_t y, const char* text, uint8_t max_len);

// Update view offset for smooth scrolling
void graphics_update_view(ViewOffset* offset, const Cursor* cursor);

// Show the letter input overlay
void graphics_show_letter_input(uint8_t selected_letter);

// Hide the letter input overlay
void graphics_hide_letter_input(void);

// Draw full clue popup with answer cells at bottom
void graphics_draw_full_clue(const Clue* clue, Direction dir, const Puzzle* puzzle, uint8_t word_pos);

// Update just the answer row in clue view (after letter entry)
void graphics_update_clue_answer(const Clue* clue, Direction dir, const Puzzle* puzzle, uint8_t word_pos);

// Helper to convert ASCII to tile index
uint8_t char_to_tile(char c);

#endif // GRAPHICS_H
