#include "../../firmware/src/hal/input_hal.h"
#include <SDL2/SDL.h>

void input_hal_init(void) {}

// F1 = PRIMARY button, F2 = SECONDARY button.
// SDL_GetKeyboardState() reflects state after the last SDL_PumpEvents().
bool input_hal_is_held(InputButton btn) {
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    if (btn == INPUT_BTN_PRIMARY)   return keys[SDL_SCANCODE_F1] != 0;
    if (btn == INPUT_BTN_SECONDARY) return keys[SDL_SCANCODE_F2] != 0;
    return false;
}
