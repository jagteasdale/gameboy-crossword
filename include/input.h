#ifndef INPUT_H
#define INPUT_H

#include <gb/gb.h>

// Input state tracking
extern uint8_t joy_current;
extern uint8_t joy_previous;
extern uint8_t joy_pressed;
extern uint8_t joy_released;

// Initialize input system
void input_init(void);

// Update input state (call once per frame)
void input_update(void);

// Check if a button was just pressed this frame
uint8_t input_just_pressed(uint8_t button);

// Check if a button is currently held
uint8_t input_held(uint8_t button);

// Check if a button was just released
uint8_t input_just_released(uint8_t button);

#endif // INPUT_H
