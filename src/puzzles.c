#include <string.h>
#include "puzzles.h"

// Sample cryptic crossword clues
static const char clue_1a[] = "Poorly made dial exploded";
static const char clue_4a[] = "Second letter from Greece";
static const char clue_7a[] = "Greeting from aloha waii";

static const char clue_1d[] = "Crazy person in the attic";
static const char clue_2d[] = "Worker insect at picnic";
static const char clue_3d[] = "Writing tool for authors";

// Sample clue arrays
static const Clue sample_across_clues[] = {
    { 1, 0, 0, 5, clue_1a },
    { 4, 0, 2, 4, clue_4a },
    { 7, 0, 4, 5, clue_7a },
};

static const Clue sample_down_clues[] = {
    { 1, 0, 0, 5, clue_1d },
    { 2, 2, 0, 5, clue_2d },
    { 3, 4, 0, 5, clue_3d },
};

// Sample 15x15 grid template
// B = black cell, . = empty white cell
// This is a simplified demo grid - real crosswords have symmetric patterns
static const char sample_grid_template[GRID_HEIGHT][GRID_WIDTH + 1] = {
    "RADIALBBBBWORDS",
    ".B.B.BBBBB.B.B.",
    "BETABBBBBBANTBB",
    ".B.BBBBBBB.B.BB",
    "ALOHABBBBBBPENB",
    "BBBBBBBBBBBBBBB",
    "...............",
    "B.B.B.B.B.B.B.B",
    "...............",
    "BBBBBBBBBBBBBBB",
    "...............",
    "B.B.B.B.B.B.B.B",
    "...............",
    "BBBBBBBBBBBBBBB",
    "...............",
};

// Convert grid template to puzzle format
static void init_grid_from_template(Cell grid[GRID_HEIGHT][GRID_WIDTH],
                                    const char template[GRID_HEIGHT][GRID_WIDTH + 1]) {
    uint8_t clue_num = 1;

    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            char c = template[y][x];
            Cell* cell = &grid[y][x];

            cell->player_input = CELL_EMPTY;
            cell->flags = 0;

            if (c == 'B' || c == '#') {
                cell->solution = CELL_BLACK;
                cell->clue_num = 0;
            } else if (c == '.') {
                cell->solution = CELL_EMPTY;  // Unknown solution
                cell->clue_num = 0;
            } else if (c >= 'A' && c <= 'Z') {
                cell->solution = c;
                cell->clue_num = 0;

                // Check if this is start of a word (for clue numbering)
                uint8_t is_across_start = (x == 0 || grid[y][x-1].solution == CELL_BLACK) &&
                                          (x + 1 < GRID_WIDTH && template[y][x+1] != 'B');
                uint8_t is_down_start = (y == 0 || grid[y-1][x].solution == CELL_BLACK) &&
                                        (y + 1 < GRID_HEIGHT && template[y+1][x] != 'B');

                if (is_across_start) cell->flags |= FLAG_ACROSS_START;
                if (is_down_start) cell->flags |= FLAG_DOWN_START;

                if (is_across_start || is_down_start) {
                    cell->clue_num = clue_num++;
                }
            }
        }
    }
}

// Sample puzzle data
static Puzzle sample_puzzle = {
    .title = "Sample Puzzle",
    .across_count = 3,
    .down_count = 3,
    .across_clues = sample_across_clues,
    .down_clues = sample_down_clues,
};

const Puzzle* puzzles_get(uint8_t index) {
    if (index == 0) {
        // Initialize grid from template on first access
        static uint8_t initialized = 0;
        if (!initialized) {
            init_grid_from_template(sample_puzzle.grid, sample_grid_template);
            initialized = 1;
        }
        return &sample_puzzle;
    }
    return NULL;
}

const char* puzzles_get_title(uint8_t index) {
    if (index == 0) {
        return sample_puzzle.title;
    }
    return "Unknown";
}

void puzzles_init_player_grid(Puzzle* dest, uint8_t puzzle_index) {
    const Puzzle* src = puzzles_get(puzzle_index);
    if (src == NULL) return;

    // Copy puzzle structure
    memcpy(dest, src, sizeof(Puzzle));

    // Clear player inputs
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (dest->grid[y][x].solution != CELL_BLACK) {
                dest->grid[y][x].player_input = CELL_EMPTY;
            }
        }
    }
}
