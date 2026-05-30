#include "board.h"
#include <Arduino.h>

extern "C" void board_init(void) {
    // No I2C bus on the CYD — nothing to bring up before display init.
    // SPI is initialised inside display_hal_init() via the GFX library.
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);
}
