#include "input.h"

uint8_t joy_current = 0;
uint8_t joy_previous = 0;
uint8_t joy_pressed = 0;
uint8_t joy_released = 0;

void input_init(void) {
    joy_current = 0;
    joy_previous = 0;
    joy_pressed = 0;
    joy_released = 0;
}

void input_update(void) {
    joy_previous = joy_current;
    joy_current = joypad();

    // Buttons that were just pressed this frame
    joy_pressed = joy_current & ~joy_previous;

    // Buttons that were just released this frame
    joy_released = ~joy_current & joy_previous;
}

uint8_t input_just_pressed(uint8_t button) {
    return (joy_pressed & button) != 0;
}

uint8_t input_held(uint8_t button) {
    return (joy_current & button) != 0;
}

uint8_t input_just_released(uint8_t button) {
    return (joy_released & button) != 0;
}
