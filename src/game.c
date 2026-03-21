#include <gb/gb.h>
#include <string.h>

#include "game.h"
#include "input.h"
#include "graphics.h"
#include "puzzles.h"
#include "save.h"

// Game state
static GameState current_state;
static Puzzle current_puzzle;
static Cursor cursor;
static ViewOffset view_offset;

// Clue-first navigation state
static uint8_t current_clue_index;  // Index into across_clues or down_clues
static uint8_t word_offset;         // Position within current word (0 = first letter)
static uint8_t current_puzzle_index; // Which puzzle is currently loaded

// Menu state
static uint8_t menu_selection;
static uint8_t pause_menu_selection;

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
static void update_cursor_from_clue(void);
static void next_clue(void);
static void prev_clue(void);
static void move_in_word(int8_t delta);
static void toggle_direction(void);
static const Clue* get_current_clue(void);
static void enter_letter(uint8_t letter);
static void delete_letter(void);
static void autosave(void);

void game_init(void) {
    current_state = STATE_TITLE;
    cursor.x = 0;
    cursor.y = 0;
    cursor.dir = DIR_ACROSS;
    view_offset.x = 0;
    view_offset.y = 0;
    current_clue_index = 0;
    word_offset = 0;
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
    current_puzzle_index = puzzle_index;
    puzzles_init_player_grid(&current_puzzle, puzzle_index);

    // Try to load saved progress
    uint8_t saved_clue, saved_offset, saved_dir;
    if (save_load(puzzle_index, &current_puzzle, &saved_clue, &saved_offset, &saved_dir)) {
        // Restore saved position
        current_clue_index = saved_clue;
        word_offset = saved_offset;
        cursor.dir = saved_dir;
    } else {
        // No save - start at first across clue
        cursor.dir = DIR_ACROSS;
        current_clue_index = 0;
        word_offset = 0;
    }

    view_offset.x = 0;
    view_offset.y = 0;

    // Set cursor position from current clue
    update_cursor_from_clue();

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
    uint8_t needs_redraw = 0;

    // UP/DOWN - cycle through clues
    if (input_just_pressed(J_UP)) {
        prev_clue();
        needs_redraw = 1;
    }
    if (input_just_pressed(J_DOWN)) {
        next_clue();
        needs_redraw = 1;
    }

    // LEFT/RIGHT - move within current word
    if (input_just_pressed(J_LEFT)) {
        move_in_word(-1);
        needs_redraw = 1;
    }
    if (input_just_pressed(J_RIGHT)) {
        move_in_word(1);
        needs_redraw = 1;
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

    // B button - delete letter and move back
    if (input_just_pressed(J_B)) {
        delete_letter();
        move_in_word(-1);
        autosave();
        needs_redraw = 1;
    }

    // SELECT - toggle direction (across/down)
    if (input_just_pressed(J_SELECT)) {
        toggle_direction();
        needs_redraw = 1;
    }

    // START - pause menu
    if (input_just_pressed(J_START)) {
        current_state = STATE_PAUSED;
        pause_menu_selection = PAUSE_OPTION_RESUME;
        graphics_draw_pause_menu(pause_menu_selection);
    }

    // Update display if anything changed
    if (needs_redraw || old_x != cursor.x || old_y != cursor.y) {
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
        move_in_word(1);  // Advance to next cell in word
        current_state = STATE_PLAYING;

        // Auto-save progress
        autosave();

        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);

        // Check for completion
        if (game_check_complete()) {
            save_mark_complete(current_puzzle_index);
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
    // Navigate menu
    if (input_just_pressed(J_UP)) {
        if (pause_menu_selection > 0) {
            pause_menu_selection--;
            graphics_draw_pause_menu(pause_menu_selection);
        }
    }
    if (input_just_pressed(J_DOWN)) {
        if (pause_menu_selection < PAUSE_OPTION_COUNT - 1) {
            pause_menu_selection++;
            graphics_draw_pause_menu(pause_menu_selection);
        }
    }

    // Select option
    if (input_just_pressed(J_A) || input_just_pressed(J_START)) {
        if (pause_menu_selection == PAUSE_OPTION_RESUME) {
            current_state = STATE_PLAYING;
            graphics_draw_grid(&current_puzzle, &view_offset);
            graphics_draw_cursor(&cursor, &view_offset);
            const Clue* clue = get_current_clue();
            if (clue) {
                graphics_draw_clue(clue, cursor.dir);
            }
        } else if (pause_menu_selection == PAUSE_OPTION_QUIT) {
            // Save progress and return to menu
            autosave();
            current_state = STATE_MENU;
            graphics_draw_menu(menu_selection, PUZZLE_COUNT);
        }
    }

    // B also resumes
    if (input_just_pressed(J_B)) {
        current_state = STATE_PLAYING;
        graphics_draw_grid(&current_puzzle, &view_offset);
        graphics_draw_cursor(&cursor, &view_offset);
        const Clue* clue = get_current_clue();
        if (clue) {
            graphics_draw_clue(clue, cursor.dir);
        }
    }
}

static void handle_complete_state(void) {
    if (input_just_pressed(J_A) || input_just_pressed(J_START)) {
        current_state = STATE_MENU;
        graphics_draw_menu(menu_selection, PUZZLE_COUNT);
    }
}

// Helper functions

static const Clue* get_current_clue(void) {
    const Clue* clues = (cursor.dir == DIR_ACROSS) ?
                        current_puzzle.across_clues :
                        current_puzzle.down_clues;
    uint8_t count = (cursor.dir == DIR_ACROSS) ?
                    current_puzzle.across_count :
                    current_puzzle.down_count;

    if (current_clue_index < count) {
        return &clues[current_clue_index];
    }
    return NULL;
}

static void update_cursor_from_clue(void) {
    const Clue* clue = get_current_clue();
    if (clue == NULL) return;

    // Clamp word_offset to valid range
    if (word_offset >= clue->length) {
        word_offset = clue->length - 1;
    }

    // Calculate cursor position from clue start + offset
    if (cursor.dir == DIR_ACROSS) {
        cursor.x = clue->start_x + word_offset;
        cursor.y = clue->start_y;
    } else {
        cursor.x = clue->start_x;
        cursor.y = clue->start_y + word_offset;
    }
}

static void next_clue(void) {
    uint8_t count = (cursor.dir == DIR_ACROSS) ?
                    current_puzzle.across_count :
                    current_puzzle.down_count;

    current_clue_index++;
    if (current_clue_index >= count) {
        // Switch direction and go to first clue
        cursor.dir = (cursor.dir == DIR_ACROSS) ? DIR_DOWN : DIR_ACROSS;
        current_clue_index = 0;
    }
    word_offset = 0;
    update_cursor_from_clue();
}

static void prev_clue(void) {
    if (current_clue_index == 0) {
        // Switch direction and go to last clue
        cursor.dir = (cursor.dir == DIR_ACROSS) ? DIR_DOWN : DIR_ACROSS;
        uint8_t count = (cursor.dir == DIR_ACROSS) ?
                        current_puzzle.across_count :
                        current_puzzle.down_count;
        current_clue_index = (count > 0) ? count - 1 : 0;
    } else {
        current_clue_index--;
    }
    word_offset = 0;
    update_cursor_from_clue();
}

static void move_in_word(int8_t delta) {
    const Clue* clue = get_current_clue();
    if (clue == NULL) return;

    int8_t new_offset = (int8_t)word_offset + delta;

    // Clamp to word bounds (don't wrap)
    if (new_offset < 0) {
        new_offset = 0;
    } else if (new_offset >= clue->length) {
        new_offset = clue->length - 1;
    }

    word_offset = new_offset;
    update_cursor_from_clue();
}

static void toggle_direction(void) {
    // Switch direction
    cursor.dir = (cursor.dir == DIR_ACROSS) ? DIR_DOWN : DIR_ACROSS;

    // Reset to first clue of new direction
    current_clue_index = 0;
    word_offset = 0;
    update_cursor_from_clue();
}

static void enter_letter(uint8_t letter) {
    current_puzzle.grid[cursor.y][cursor.x].player_input = letter;
}

static void delete_letter(void) {
    current_puzzle.grid[cursor.y][cursor.x].player_input = CELL_EMPTY;
}

static void autosave(void) {
    save_puzzle(current_puzzle_index, &current_puzzle,
                current_clue_index, word_offset, cursor.dir);
}
