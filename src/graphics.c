#include <gb/gb.h>
#include <string.h>

#include "graphics.h"

// External tile data from assets
extern const uint8_t tile_data[];
extern const uint8_t cursor_sprite_data[];
#define TILE_DATA_COUNT 83
#define CURSOR_SPRITE_COUNT 1

// View configuration
// Grid area: top portion of screen (rows 0-14 = 15 tiles)
// Clue area: bottom portion of screen (rows 15-17 = 3 tiles)
#define GRID_VIEW_HEIGHT 15
#define CLUE_AREA_Y 15
#define GRID_VIEW_WIDTH 15

// Screen buffer for efficient updates
static uint8_t screen_buffer[SCREEN_TILES_Y][SCREEN_TILES_X];

// Convert ASCII character to tile index
uint8_t char_to_tile(char c) {
    if (c >= 'A' && c <= 'Z') return TILE_LETTER_A + (c - 'A');
    if (c >= 'a' && c <= 'z') return TILE_LETTER_A + (c - 'a');
    if (c >= '0' && c <= '9') return TILE_NUMBER_0 + (c - '0');
    switch (c) {
        case ':': return TILE_COLON;
        case '.': return TILE_PERIOD;
        case '-': return TILE_HYPHEN;
        case ',': return 0x3D;  // TILE_COMMA
        case '?': return 0x3E;  // TILE_QUESTION
        case '\'': return 0x3F; // TILE_APOSTROPHE
        case '(': return 0x41;  // TILE_OPEN_PAREN
        case ')': return 0x42;  // TILE_CLOSE_PAREN
        case '!': return 0x43;  // TILE_EXCLAIM
        default: return TILE_SPACE;
    }
}

void graphics_init(void) {
    // Disable LCD for safe VRAM access (wait for vblank first)
    while (STAT_REG & STATF_BUSY);
    LCDC_REG = 0;

    // Set palettes
    BGP_REG = 0xE4;
    OBP0_REG = 0xE4;
    OBP1_REG = 0xD2;

    // Load tiles
    graphics_load_tiles();

    // Clear screen buffer
    for (uint8_t y = 0; y < SCREEN_TILES_Y; y++) {
        for (uint8_t x = 0; x < SCREEN_TILES_X; x++) {
            screen_buffer[y][x] = TILE_SPACE;
        }
    }

    // Enable LCD with proper settings
    LCDC_REG = LCDCF_ON | LCDCF_BG8000 | LCDCF_BG9800 | LCDCF_BGON | LCDCF_OBJ8 | LCDCF_OBJON;
}

void graphics_load_tiles(void) {
    // Load all tile data from assets
    set_bkg_data(0, TILE_DATA_COUNT, tile_data);

    // Load cursor sprite data for sprite use
    set_sprite_data(0, TILE_DATA_COUNT, tile_data);
}

void graphics_draw_grid(const Puzzle* puzzle, const ViewOffset* offset) {
    // Draw visible portion of the grid
    for (uint8_t vy = 0; vy < GRID_VIEW_HEIGHT; vy++) {
        for (uint8_t vx = 0; vx < GRID_VIEW_WIDTH; vx++) {
            uint8_t grid_x = vx + offset->x;
            uint8_t grid_y = vy + offset->y;
            uint8_t tile;

            if (grid_x < GRID_WIDTH && grid_y < GRID_HEIGHT) {
                const Cell* cell = &puzzle->grid[grid_y][grid_x];

                if (cell->solution == CELL_BLACK) {
                    tile = TILE_BLACK;
                } else if (cell->player_input >= 'A' && cell->player_input <= 'Z') {
                    tile = TILE_LETTER_A + (cell->player_input - 'A');
                } else {
                    tile = TILE_EMPTY;
                }
            } else {
                tile = TILE_BLACK;  // Out of bounds = black
            }

            screen_buffer[vy][vx] = tile;
        }

        // Fill remaining columns with black (border)
        for (uint8_t vx = GRID_VIEW_WIDTH; vx < SCREEN_TILES_X; vx++) {
            screen_buffer[vy][vx] = TILE_BLACK;
        }
    }

    // Copy grid area to VRAM
    set_bkg_tiles(0, 0, SCREEN_TILES_X, GRID_VIEW_HEIGHT, (uint8_t*)screen_buffer);
}

void graphics_draw_cursor(const Cursor* cursor, const ViewOffset* offset) {
    // Calculate screen position
    uint8_t screen_x = cursor->x - offset->x;
    uint8_t screen_y = cursor->y - offset->y;

    // Only draw if on screen
    if (screen_x < GRID_VIEW_WIDTH && screen_y < GRID_VIEW_HEIGHT) {
        // Sprite positions are offset by 8,16 from tile positions
        move_sprite(0, (screen_x * 8) + 8, (screen_y * 8) + 16);
    } else {
        // Hide sprite if cursor not visible
        move_sprite(0, 0, 0);
    }
}

void graphics_clear_cursor(uint8_t x, uint8_t y, const ViewOffset* offset) {
    // Hide cursor sprite
    move_sprite(0, 0, 0);
}

void graphics_draw_clue(const Clue* clue, Direction dir) {
    // Clear clue area (3 rows at bottom)
    for (uint8_t y = CLUE_AREA_Y; y < SCREEN_TILES_Y; y++) {
        for (uint8_t x = 0; x < SCREEN_TILES_X; x++) {
            screen_buffer[y][x] = TILE_SPACE;
        }
    }

    if (clue == NULL || clue->text == NULL) {
        set_bkg_tiles(0, CLUE_AREA_Y, SCREEN_TILES_X, 3, (uint8_t*)&screen_buffer[CLUE_AREA_Y]);
        return;
    }

    // Draw clue number and direction (e.g., "1A:" or "2D:")
    uint8_t x_pos = 0;

    // Draw number
    if (clue->number >= 10) {
        screen_buffer[CLUE_AREA_Y][x_pos++] = TILE_NUMBER_0 + (clue->number / 10);
    }
    screen_buffer[CLUE_AREA_Y][x_pos++] = TILE_NUMBER_0 + (clue->number % 10);

    // Draw direction
    screen_buffer[CLUE_AREA_Y][x_pos++] = (dir == DIR_ACROSS) ?
        TILE_LETTER_A : TILE_LETTER_A + ('D' - 'A');
    screen_buffer[CLUE_AREA_Y][x_pos++] = TILE_COLON;
    x_pos++;  // Space

    // Draw clue text with word wrap
    const char* text = clue->text;
    uint8_t row = CLUE_AREA_Y;

    while (*text != '\0' && row < SCREEN_TILES_Y) {
        if (x_pos >= SCREEN_TILES_X) {
            // Wrap to next line
            row++;
            x_pos = 0;
            if (row >= SCREEN_TILES_Y) break;
        }

        screen_buffer[row][x_pos] = char_to_tile(*text);
        x_pos++;
        text++;
    }

    // Copy clue area to VRAM
    set_bkg_tiles(0, CLUE_AREA_Y, SCREEN_TILES_X, 3, (uint8_t*)&screen_buffer[CLUE_AREA_Y]);
}

void graphics_draw_title(void) {
    // Clear entire screen
    for (uint8_t y = 0; y < SCREEN_TILES_Y; y++) {
        for (uint8_t x = 0; x < SCREEN_TILES_X; x++) {
            screen_buffer[y][x] = TILE_SPACE;
        }
    }

    // Draw "CROSSWORD" title (centered at row 6)
    const char* title = "CROSSWORD";
    uint8_t title_x = (SCREEN_TILES_X - 9) / 2;
    for (uint8_t i = 0; title[i] != '\0'; i++) {
        screen_buffer[6][title_x + i] = char_to_tile(title[i]);
    }

    // Draw decorative line
    for (uint8_t x = 4; x < 16; x++) {
        screen_buffer[8][x] = TILE_HYPHEN;
    }

    // Draw "PRESS START" (centered at row 12)
    const char* prompt = "PRESS START";
    uint8_t prompt_x = (SCREEN_TILES_X - 11) / 2;
    for (uint8_t i = 0; prompt[i] != '\0'; i++) {
        screen_buffer[12][prompt_x + i] = char_to_tile(prompt[i]);
    }

    // Copy to VRAM
    set_bkg_tiles(0, 0, SCREEN_TILES_X, SCREEN_TILES_Y, (uint8_t*)screen_buffer);

    // Hide cursor sprite
    move_sprite(0, 0, 0);
}

void graphics_draw_menu(uint8_t selected_index, uint8_t puzzle_count) {
    // Clear screen
    for (uint8_t y = 0; y < SCREEN_TILES_Y; y++) {
        for (uint8_t x = 0; x < SCREEN_TILES_X; x++) {
            screen_buffer[y][x] = TILE_SPACE;
        }
    }

    // Draw "SELECT PUZZLE" header
    const char* header = "SELECT PUZZLE";
    uint8_t header_x = (SCREEN_TILES_X - 13) / 2;
    for (uint8_t i = 0; header[i] != '\0'; i++) {
        screen_buffer[1][header_x + i] = char_to_tile(header[i]);
    }

    // Draw puzzle list
    for (uint8_t i = 0; i < puzzle_count && i < 8; i++) {
        uint8_t y_pos = 4 + (i * 2);

        // Draw cursor indicator for selected item
        if (i == selected_index) {
            screen_buffer[y_pos][1] = TILE_ARROW_RIGHT;
        }

        // Draw puzzle number and title
        screen_buffer[y_pos][3] = TILE_NUMBER_0 + (i + 1);
        screen_buffer[y_pos][4] = TILE_PERIOD;

        // Draw "PUZZLE N" (placeholder - would use actual title)
        const char* puzzle_text = "PUZZLE";
        for (uint8_t j = 0; puzzle_text[j] != '\0'; j++) {
            screen_buffer[y_pos][6 + j] = char_to_tile(puzzle_text[j]);
        }
    }

    // Copy to VRAM
    set_bkg_tiles(0, 0, SCREEN_TILES_X, SCREEN_TILES_Y, (uint8_t*)screen_buffer);

    // Hide cursor sprite
    move_sprite(0, 0, 0);
}

void graphics_draw_complete(void) {
    // Clear screen
    for (uint8_t y = 0; y < SCREEN_TILES_Y; y++) {
        for (uint8_t x = 0; x < SCREEN_TILES_X; x++) {
            screen_buffer[y][x] = TILE_SPACE;
        }
    }

    // Draw celebratory message
    const char* complete = "COMPLETE!";
    uint8_t complete_x = (SCREEN_TILES_X - 9) / 2;
    for (uint8_t i = 0; complete[i] != '\0'; i++) {
        screen_buffer[7][complete_x + i] = char_to_tile(complete[i]);
    }

    const char* well_done = "WELL DONE";
    uint8_t wd_x = (SCREEN_TILES_X - 9) / 2;
    for (uint8_t i = 0; well_done[i] != '\0'; i++) {
        screen_buffer[9][wd_x + i] = char_to_tile(well_done[i]);
    }

    const char* prompt = "PRESS START";
    uint8_t prompt_x = (SCREEN_TILES_X - 11) / 2;
    for (uint8_t i = 0; prompt[i] != '\0'; i++) {
        screen_buffer[13][prompt_x + i] = char_to_tile(prompt[i]);
    }

    // Copy to VRAM
    set_bkg_tiles(0, 0, SCREEN_TILES_X, SCREEN_TILES_Y, (uint8_t*)screen_buffer);

    // Hide cursor sprite
    move_sprite(0, 0, 0);
}

void graphics_draw_pause_menu(void) {
    // Clear screen
    for (uint8_t y = 0; y < SCREEN_TILES_Y; y++) {
        for (uint8_t x = 0; x < SCREEN_TILES_X; x++) {
            screen_buffer[y][x] = TILE_SPACE;
        }
    }

    // Draw "PAUSED" (centered at row 7)
    const char* paused = "PAUSED";
    uint8_t paused_x = (SCREEN_TILES_X - 6) / 2;
    for (uint8_t i = 0; paused[i] != '\0'; i++) {
        screen_buffer[7][paused_x + i] = char_to_tile(paused[i]);
    }

    // Draw "PRESS START" (centered at row 11)
    const char* prompt = "PRESS START";
    uint8_t prompt_x = (SCREEN_TILES_X - 11) / 2;
    for (uint8_t i = 0; prompt[i] != '\0'; i++) {
        screen_buffer[11][prompt_x + i] = char_to_tile(prompt[i]);
    }

    // Copy to VRAM
    set_bkg_tiles(0, 0, SCREEN_TILES_X, SCREEN_TILES_Y, (uint8_t*)screen_buffer);

    // Hide cursor sprite
    move_sprite(0, 0, 0);
}

void graphics_draw_cell(uint8_t screen_x, uint8_t screen_y, const Cell* cell, uint8_t is_cursor) {
    uint8_t tile;

    if (cell->solution == CELL_BLACK) {
        tile = TILE_BLACK;
    } else if (cell->player_input >= 'A' && cell->player_input <= 'Z') {
        tile = TILE_LETTER_A + (cell->player_input - 'A');
    } else {
        tile = TILE_EMPTY;
    }

    set_bkg_tile_xy(screen_x, screen_y, tile);
}

void graphics_draw_text(uint8_t x, uint8_t y, const char* text, uint8_t max_len) {
    uint8_t i = 0;
    while (text[i] != '\0' && i < max_len && (x + i) < SCREEN_TILES_X) {
        set_bkg_tile_xy(x + i, y, char_to_tile(text[i]));
        i++;
    }
}

void graphics_update_view(ViewOffset* offset, const Cursor* cursor) {
    // Keep cursor visible with margin from edges
    const uint8_t MARGIN = 3;

    // Horizontal scrolling
    if (cursor->x < offset->x + MARGIN) {
        offset->x = (cursor->x >= MARGIN) ? cursor->x - MARGIN : 0;
    } else if (cursor->x >= offset->x + GRID_VIEW_WIDTH - MARGIN) {
        offset->x = cursor->x - GRID_VIEW_WIDTH + MARGIN + 1;
        if (offset->x + GRID_VIEW_WIDTH > GRID_WIDTH) {
            offset->x = (GRID_WIDTH > GRID_VIEW_WIDTH) ? GRID_WIDTH - GRID_VIEW_WIDTH : 0;
        }
    }

    // Vertical scrolling
    if (cursor->y < offset->y + MARGIN) {
        offset->y = (cursor->y >= MARGIN) ? cursor->y - MARGIN : 0;
    } else if (cursor->y >= offset->y + GRID_VIEW_HEIGHT - MARGIN) {
        offset->y = cursor->y - GRID_VIEW_HEIGHT + MARGIN + 1;
        if (offset->y + GRID_VIEW_HEIGHT > GRID_HEIGHT) {
            offset->y = (GRID_HEIGHT > GRID_VIEW_HEIGHT) ? GRID_HEIGHT - GRID_VIEW_HEIGHT : 0;
        }
    }
}

// Letter picker layout: 9 columns x 3 rows = 27 cells (26 letters + 1 blank)
#define PICKER_COLS 9
#define PICKER_ROWS 3
#define PICKER_X 1      // Starting X position on screen
#define PICKER_Y 7      // Starting Y position on screen
#define PICKER_WIDTH 11 // Total width including border
#define PICKER_HEIGHT 5 // Total height including border

// Saved background tiles for restoration
static uint8_t saved_tiles[PICKER_HEIGHT][PICKER_WIDTH];
static uint8_t picker_visible = 0;

void graphics_show_letter_input(uint8_t selected_letter) {
    uint8_t row_buffer[PICKER_WIDTH];

    // Save the background tiles we're about to overwrite (only once)
    if (!picker_visible) {
        for (uint8_t y = 0; y < PICKER_HEIGHT; y++) {
            for (uint8_t x = 0; x < PICKER_WIDTH; x++) {
                saved_tiles[y][x] = screen_buffer[PICKER_Y + y][PICKER_X + x];
            }
        }
        picker_visible = 1;
    }

    // Draw top border
    row_buffer[0] = TILE_PERIOD;
    for (uint8_t x = 1; x < PICKER_WIDTH - 1; x++) {
        row_buffer[x] = TILE_HYPHEN;
    }
    row_buffer[PICKER_WIDTH - 1] = TILE_PERIOD;
    set_bkg_tiles(PICKER_X, PICKER_Y, PICKER_WIDTH, 1, row_buffer);

    // Draw letter rows
    for (uint8_t row = 0; row < PICKER_ROWS; row++) {
        row_buffer[0] = TILE_SPACE;  // Left padding

        for (uint8_t col = 0; col < PICKER_COLS; col++) {
            uint8_t letter_index = row * PICKER_COLS + col;
            if (letter_index < 26) {
                row_buffer[1 + col] = TILE_LETTER_A + letter_index;
            } else {
                row_buffer[1 + col] = TILE_SPACE;
            }
        }

        row_buffer[PICKER_WIDTH - 1] = TILE_SPACE;  // Right padding
        set_bkg_tiles(PICKER_X, PICKER_Y + 1 + row, PICKER_WIDTH, 1, row_buffer);
    }

    // Draw bottom border
    row_buffer[0] = TILE_PERIOD;
    for (uint8_t x = 1; x < PICKER_WIDTH - 1; x++) {
        row_buffer[x] = TILE_HYPHEN;
    }
    row_buffer[PICKER_WIDTH - 1] = TILE_PERIOD;
    set_bkg_tiles(PICKER_X, PICKER_Y + PICKER_HEIGHT - 1, PICKER_WIDTH, 1, row_buffer);

    // Position cursor sprite on selected letter
    uint8_t sel_row = selected_letter / PICKER_COLS;
    uint8_t sel_col = selected_letter % PICKER_COLS;
    // Sprite coords: tile position * 8, plus 8 for X offset, 16 for Y offset
    uint8_t sprite_x = (PICKER_X + 1 + sel_col) * 8 + 8;
    uint8_t sprite_y = (PICKER_Y + 1 + sel_row) * 8 + 16;

    set_sprite_tile(0, TILE_CURSOR);
    move_sprite(0, sprite_x, sprite_y);
}

void graphics_hide_letter_input(void) {
    if (!picker_visible) return;

    // Restore saved tiles row by row
    for (uint8_t y = 0; y < PICKER_HEIGHT; y++) {
        set_bkg_tiles(PICKER_X, PICKER_Y + y, PICKER_WIDTH, 1, saved_tiles[y]);
        // Also restore screen_buffer for consistency
        for (uint8_t x = 0; x < PICKER_WIDTH; x++) {
            screen_buffer[PICKER_Y + y][PICKER_X + x] = saved_tiles[y][x];
        }
    }

    picker_visible = 0;

    // Hide cursor sprite (will be repositioned by draw_cursor)
    move_sprite(0, 0, 0);
}
