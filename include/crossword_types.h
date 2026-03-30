#ifndef CROSSWORD_TYPES_H
#define CROSSWORD_TYPES_H

#include <gb/gb.h>

// Game Boy screen dimensions (in tiles)
#define SCREEN_TILES_X 20
#define SCREEN_TILES_Y 18

// Crossword grid dimensions
#define GRID_WIDTH 15
#define GRID_HEIGHT 15

// Cell states
#define CELL_BLACK 0xFF    // Blocked cell
#define CELL_EMPTY 0x00    // Empty white cell
// Values 'A'-'Z' (65-90) represent filled letters

// Direction for clue navigation
typedef enum {
    DIR_ACROSS = 0,
    DIR_DOWN = 1
} Direction;

// Cell structure for the crossword grid
typedef struct {
    uint8_t solution;      // Correct letter (or CELL_BLACK)
    uint8_t player_input;  // Player's current input
    uint8_t clue_num;      // Clue number (0 if none)
    uint8_t flags;         // Bit flags for cell properties
} Cell;

// Flag bits for Cell.flags
#define FLAG_ACROSS_START 0x01  // Start of an across word
#define FLAG_DOWN_START   0x02  // Start of a down word
#define FLAG_REVEALED     0x04  // Cell was revealed (hint used)
#define FLAG_CHECKED      0x08  // Cell was checked by player

// Clue structure
typedef struct {
    uint8_t number;        // Clue number
    uint8_t start_x;       // Starting X position
    uint8_t start_y;       // Starting Y position
    uint8_t length;        // Word length
    const char* text;      // Clue text (pointer to ROM)
} Clue;

// Crossword puzzle structure
typedef struct {
    const char* title;             // Puzzle title
    Cell grid[GRID_HEIGHT][GRID_WIDTH];
    uint8_t across_count;          // Number of across clues
    uint8_t down_count;            // Number of down clues
    const Clue* across_clues;      // Array of across clues
    const Clue* down_clues;        // Array of down clues
} Puzzle;

// Game state
typedef enum {
    STATE_TITLE,
    STATE_MENU,
    STATE_PLAYING,
    STATE_CLUE_VIEW,
    STATE_LETTER_INPUT,
    STATE_PAUSED,
    STATE_CHEAT_MENU,
    STATE_CONFIRM_CLEAR,
    STATE_COMPLETE
} GameState;

// Player cursor position
typedef struct {
    uint8_t x;
    uint8_t y;
    Direction dir;
} Cursor;

// View offset for scrolling the grid
typedef struct {
    uint8_t x;
    uint8_t y;
} ViewOffset;

#endif // CROSSWORD_TYPES_H
