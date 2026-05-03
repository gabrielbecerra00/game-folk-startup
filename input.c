#include "input.h"

void input_update(ButtonState *state)
{
    uint32_t pins = DL_GPIO_readPins(BUTTONS_PORT,
                                     BUTTONS_BTN1_PIN |  // Up
                                     BUTTONS_BTN2_PIN |  // Right
                                     BUTTONS_BTN3_PIN |  // Down
                                     BUTTONS_BTN4_PIN |  // Left
                                     BUTTONS_BTN5_PIN |  // D
                                     BUTTONS_BTN6_PIN |  // C
                                     BUTTONS_BTN7_PIN |  // B
                                     BUTTONS_BTN8_PIN);  // A

    state->up    = !(pins & BUTTONS_BTN1_PIN);
    state->right = !(pins & BUTTONS_BTN2_PIN);
    state->down  = !(pins & BUTTONS_BTN3_PIN);
    state->left  = !(pins & BUTTONS_BTN4_PIN);
    state->d     = !(pins & BUTTONS_BTN5_PIN);
    state->c     = !(pins & BUTTONS_BTN6_PIN);
    state->b     = !(pins & BUTTONS_BTN7_PIN);
    state->a     = !(pins & BUTTONS_BTN8_PIN);
}
