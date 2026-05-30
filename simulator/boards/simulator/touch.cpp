#include "../../firmware/src/hal/touch_hal.h"
#include <SDL2/SDL.h>

void touch_hal_init(void) {}

// SDL_GetMouseState() is safe to call from any point after SDL_Init().
// Coordinates are already in display space (no axis swap needed).
void touch_hal_read(uint16_t* x, uint16_t* y, bool* pressed) {
    int mx, my;
    uint32_t buttons = SDL_GetMouseState(&mx, &my);
    *pressed = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    *x = (uint16_t)(mx < 0 ? 0 : mx);
    *y = (uint16_t)(my < 0 ? 0 : my);
}
