#include "../../hal/touch_hal.h"

// XPT2046 touch is stubbed for now (visual-only mode).
// To enable: add XPT2046_Touchscreen library and implement here.
void touch_hal_init(void) {}

void touch_hal_read(uint16_t* x, uint16_t* y, bool* pressed) {
    *x = 0;
    *y = 0;
    *pressed = false;
}
