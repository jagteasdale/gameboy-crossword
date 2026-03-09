#include <gb/gb.h>
#include <stdio.h>

#include "crossword_types.h"
#include "game.h"
#include "input.h"
#include "graphics.h"

void main(void) {
    // Disable interrupts during initialization
    disable_interrupts();

    // Initialize subsystems
    input_init();
    graphics_init();
    game_init();

    // Enable display
    DISPLAY_ON;

    // Enable interrupts
    enable_interrupts();

    // Main game loop
    while(1) {
        // Wait for vblank
        wait_vbl_done();

        // Update input state
        input_update();

        // Update game logic
        game_update();
    }
}
