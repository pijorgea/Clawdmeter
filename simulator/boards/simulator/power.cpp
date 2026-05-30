#include "../../firmware/src/hal/power_hal.h"
#include <SDL2/SDL.h>

void power_hal_init(void) {}
void power_hal_tick(void) {}

// Simulate a healthy battery, not charging, USB connected (prevents idle sleep).
int  power_hal_battery_pct(void)  { return 80; }
bool power_hal_is_charging(void)  { return false; }
bool power_hal_is_vbus_in(void)   { return true; }

// F3 key maps to the PWR button. Edge-triggered: true only on press transition.
bool power_hal_pwr_pressed(void) {
    static bool prev = false;
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    bool now = keys[SDL_SCANCODE_F3] != 0;
    bool edge = now && !prev;
    prev = now;
    return edge;
}
