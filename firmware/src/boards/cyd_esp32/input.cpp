#include "../../hal/input_hal.h"
#include "board.h"
#include <Arduino.h>

void input_hal_init(void) {
    pinMode(BTN_BOOT, INPUT_PULLUP);
}

// BOOT button (active LOW) = PRIMARY. No secondary button on CYD.
bool input_hal_is_held(InputButton btn) {
    if (btn == INPUT_BTN_PRIMARY) return digitalRead(BTN_BOOT) == LOW;
    return false;
}
