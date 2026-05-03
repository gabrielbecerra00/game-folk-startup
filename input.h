#ifndef INPUT_H
#define INPUT_H

#include "ti_msp_dl_config.h"
#include <stdbool.h>

// Button states (active low — 0 = pressed)
typedef struct {
    bool left;
    bool right;
    bool up;
    bool down;
    bool a;
    bool b;
    bool c;
    bool d;
} ButtonState;

void input_update(ButtonState *state);

#endif
