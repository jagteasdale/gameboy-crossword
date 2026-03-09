#include <gb/gb.h>
#include <string.h>

#include "game.h"
#include "input.h"
#include "graphics.h"
#include "puzzles.h"

// Game state
static GameState current_state;
static Puzzle current_puzzle;
static Cursor cursor;
static ViewOffset view_offset;

// Menu state
static uint8_t menu_selection;

// Letter input state
static uint8_t letter_input_selection;  // 0-25 for A-Z

// Forward declarations for state handlers
static void handle_title_state(void);
static void handle_menu_state(void);
static void handle_playing_state(void);
static void handle_clue_view_state(void);
static void handle_letter_input_state(void);
static void handle_paused_state(void);
static void handle_complete_state(void);

// Helper functions
static void move_cursor(int8_t dx, int8_t dy);
static void toggle_direction(void);
static void advance_cursor(void);
static const Clue* get_current_clue(void);
static void enter_letter(uint8_t letter);
static void delete_letter(void);

void game_init(void) {
    current_state = STATE_TITLE;
    cursor.x = 0;
    cursor.y = 0;
    cursor.dir = DIR_ACROSS;
    view_offset.x = 0;
    view_offset.y = 0;
    menu_selection = 0;
    letter_input_selection = 0;

    graphics_draw_title();
}

void game_update(void) {
    switch (current_state) {
        case STATE_TITLE:
            handle_title_state();
            break;
        case STATE_MENU:
            handle_menu_state();
            break;
        case STATE_PLAYING:
            handle_playing_state();
            break;
        case STATE_CLUE_VIEW:
            handle_clue_view_state();
            break;
        case STATE_LETTER_INPUT:
            handle_letter_input_state();
            break;
        case STATE_PAUSED:
            handle_paused_state();
            break;
        case STATE_COMPLETE:
            handle_complete_state();
            break;
    }
}

void game_load_puzzle(uint8_t puzzle_index) {
    puzzles_init_player_grid(&current_puzzle, puzzle_index);

    // Find first non-black cell for cursor
    cursor.x = 0;
    cursor.y = 0;
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            if (current_puzzle.grid[y][x].solution != CELL_BLACK) {
                cursor.x = x;
                cursor.y = y;
                goto found;
            }
        }
    }
found:
    cursor.dir = DIR_ACROSS;
    view_offset.x = 0;
    view_offset.y = 0;

    // Draw initial grid
    graphics_draw_grid(&current_puzzle, &view_offset);
    graphics_draw_cursor(&cursor, &view_offset);

    // Show initial clue
    const Clue* clue = get_current_clue();
    if (clue) {
        graphics_draw_clue(clue, cursor.dir);
    }
}

uint8_t game_check_complete(void) {
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
        for (uint8_t x = 0; x < GRID_WIDTH; x++) {
            Cell* cell = &current_puzzle.grid[y][x];
            if (cell->solution != CELL_BLACK) {
                if (cell->player_input != cell->solution) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

GameState game_get_state(void) {
    return current_state;
}

void game_set_state(GameState state) {
    current_state = state;
}

// State handlers

static void handle_title_state(void) {
    if (input_just_pressed(J_START) || input_just_pressed(J_A)) {
        current_state = STATE_MENU;
        graphics_draw_menu(menu_selection, PUZZLE_COUNT);
    }
}

static void handle_menu_state(void) {
    if (input_just_pressed(J_UP)) {
        if (menu_selection > 0) {
            menu_selection--;
            graphics_draw_menu(menu_selection, PUZZLE_COUNT);
        }
    }
    if (input_just_pressed(J_DOWN)) {
        if (menu_selection < PUZZLE_COUNT - 1) {
            menu_selection++;
            graphics_draw_menu(menu_selection, PUZZLE_COUNT);
        }
    }
    if (input_just_pressed(J_A) || input_just_pressed(J_START)) {
        current_state = STATE_PLAYING;
        game_load_puzzle(menu_selection);
    }
    if (input_just_pressed(J_B)) {
        current_state = STATE_TITLE;
        graphics_draw_title();
    }
}

static void handle_playing_state(void) {
    uint8_t old_x = cursor.x;
    uint8_t old_y = cursor.y;
    uint8_t cursor_moved = 0;

    // D-pad movement
    if (input_just_pressed(J_UP)) {
        move_cursor(0, -1);
        cursor_moved = 1;
    }
    if (input_just_pressed(J_DOWN)) {
        move_cursor(0, 1);
        cursor_moved = 1;
    }
    if (input_just_pressed(J_LEFT)) {
        move_cursor(-1, 0);
        cursor_moved = 1;
    }
    if (input_just_pressed(J_RIGHT)) {
        move_cursor(1, 0);
        cursor_moved = 1;
    }

    // A button - enter letter input mode
    if (input_just_pressed(J_A)) {
        Cell* cell = &current_puzzle.grid[cursor.y][cursor.x];
        if (cell->solution != CELL_BLACK) {
            // Start at current letter or 'A'
            if (cell->player_input >= 'A' && cell->player_input <= 'Z') {
                letter_input_selection = cell->player_input - 'A';
            } else {
                letter_input_selection = 0;
            }
            current_state = STATE_LETTER_INPUT;
            graphics_show_letter_input(letter_input_selection);
        }
    }

    // B button - delete letter / go back
    if (input_just_pressed(J_B)) {
        delete_letter();
        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);
    }

    // SELECT - toggle direction
    if (input_just_pressed(J_SELECT)) {
        toggle_direction();
        cursor_moved = 1;  // Redraw clue
    }

    // START - pause menu
    if (input_just_pressed(J_START)) {
        current_state = STATE_PAUSED;
        // TODO: Draw pause menu
    }

    // Update display if cursor moved
    if (cursor_moved || old_x != cursor.x || old_y != cursor.y) {
        graphics_update_view(&view_offset, &cursor);
        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);

        const Clue* clue = get_current_clue();
        if (clue) {
            graphics_draw_clue(clue, cursor.dir);
        }
    }
}

static void handle_clue_view_state(void) {
    // Full clue view - press any button to return
    if (input_just_pressed(J_A) || input_just_pressed(J_B)) {
        current_state = STATE_PLAYING;
        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);
    }
}

static void handle_letter_input_state(void) {
    // Navigate letter selection in 9x3 grid
    #define PICKER_GRID_COLS 9

    if (input_just_pressed(J_RIGHT)) {
        // Move right, wrap within row
        uint8_t row = letter_input_selection / PICKER_GRID_COLS;
        uint8_t col = letter_input_selection % PICKER_GRID_COLS;
        col = (col + 1) % PICKER_GRID_COLS;
        uint8_t new_sel = row * PICKER_GRID_COLS + col;
        if (new_sel < 26) letter_input_selection = new_sel;
        graphics_show_letter_input(letter_input_selection);
    }
    if (input_just_pressed(J_LEFT)) {
        // Move left, wrap within row
        uint8_t row = letter_input_selection / PICKER_GRID_COLS;
        uint8_t col = letter_input_selection % PICKER_GRID_COLS;
        col = (col + PICKER_GRID_COLS - 1) % PICKER_GRID_COLS;
        uint8_t new_sel = row * PICKER_GRID_COLS + col;
        if (new_sel < 26) letter_input_selection = new_sel;
        graphics_show_letter_input(letter_input_selection);
    }
    if (input_just_pressed(J_DOWN)) {
        // Move down one row (9 letters)
        uint8_t new_sel = letter_input_selection + PICKER_GRID_COLS;
        if (new_sel >= 26) new_sel = letter_input_selection % PICKER_GRID_COLS;
        letter_input_selection = new_sel;
        graphics_show_letter_input(letter_input_selection);
    }
    if (input_just_pressed(J_UP)) {
        // Move up one row
        if (letter_input_selection >= PICKER_GRID_COLS) {
            letter_input_selection -= PICKER_GRID_COLS;
        } else {
            // Wrap to bottom row
            uint8_t col = letter_input_selection;
            letter_input_selection = 18 + col;  // Row 2 (last full row)
            if (letter_input_selection >= 26) letter_input_selection = 17 + col;
        }
        graphics_show_letter_input(letter_input_selection);
    }

    // Confirm letter
    if (input_just_pressed(J_A)) {
        enter_letter('A' + letter_input_selection);
        graphics_hide_letter_input();
        advance_cursor();
        current_state = STATE_PLAYING;

        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);

        // Check for completion
        if (game_check_complete()) {
            current_state = STATE_COMPLETE;
            graphics_draw_complete();
        }
    }

    // Cancel
    if (input_just_pressed(J_B)) {
        graphics_hide_letter_input();
        current_state = STATE_PLAYING;
        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);
    }
}

static void handle_paused_state(void) {
    if (input_just_pressed(J_START)) {
        current_state = STATE_PLAYING;
        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);
    }
}

static void handle_complete_state(void) {
    if (input_just_pressed(J_A) || input_just_pressed(J_START)) {
        current_state = STATE_MENU;
        graphics_draw_menu(menu_selection, PUZZLE_COUNT);
    }
}

// Helper functions

static void move_cursor(int8_t dx, int8_t dy) {
    int8_t new_x = cursor.x + dx;
    int8_t new_y = cursor.y + dy;

    // Bounds checking
    if (new_x < 0 || new_x >= GRID_WIDTH) return;
    if (new_y < 0 || new_y >= GRID_HEIGHT) return;

    // Skip black cells
    while (current_puzzle.grid[new_y][new_x].solution == CELL_BLACK) {
        new_x += dx;
        new_y += dy;
        if (new_x < 0 || new_x >= GRID_WIDTH) return;
        if (new_y < 0 || new_y >= GRID_HEIGHT) return;
    }

    cursor.x = new_x;
    cursor.y = new_y;
}

static void toggle_direction(void) {
    cursor.dir = (cursor.dir == DIR_ACROSS) ? DIR_DOWN : DIR_ACROSS;
}

static void advance_cursor(void) {
    if (cursor.dir == DIR_ACROSS) {
        move_cursor(1, 0);
    } else {
        move_cursor(0, 1);
    }
}

static const Clue* get_current_clue(void) {
    Cell* cell = &current_puzzle.grid[cursor.y][cursor.x];

    // Find the clue for the current cell
    const Clue* clues = (cursor.dir == DIR_ACROSS) ?
                        current_puzzle.across_clues :
                        current_puzzle.down_clues;
    uint8_t count = (cursor.dir == DIR_ACROSS) ?
                    current_puzzle.across_count :
                    current_puzzle.down_count;

    // Walk backwards to find start of word
    int8_t search_x = cursor.x;
    int8_t search_y = cursor.y;

    if (cursor.dir == DIR_ACROSS) {
        while (search_x > 0 &&
               current_puzzle.grid[search_y][search_x - 1].solution != CELL_BLACK) {
            search_x--;
        }
    } else {
        while (search_y > 0 &&
               current_puzzle.grid[search_y - 1][search_x].solution != CELL_BLACK) {
            search_y--;
        }
    }

    // Find matching clue
    for (uint8_t i = 0; i < count; i++) {
        if (clues[i].start_x == search_x && clues[i].start_y == search_y) {
            return &clues[i];
        }
    }

    return NULL;
}

static void enter_letter(uint8_t letter) {
    current_puzzle.grid[cursor.y][cursor.x].player_input = letter;
}

static void delete_letter(void) {
    current_puzzle.grid[cursor.y][cursor.x].player_input = CELL_EMPTY;
}
