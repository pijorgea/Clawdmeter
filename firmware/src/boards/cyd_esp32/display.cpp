#include "../../hal/display_hal.h"
#include "board.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

static Arduino_DataBus* bus = nullptr;
static Arduino_ILI9341* gfx = nullptr;

void display_hal_init(void) {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, LCD_MISO);
    // Rotation 1 = landscape (320x240). No MADCTL invert needed for CYD.
    gfx = new Arduino_ILI9341(bus, LCD_RST, 1 /* rotation */, false);
}

void display_hal_begin(void) {
    gfx->begin(40000000UL);
    gfx->fillScreen(0x0000);
}

void display_hal_set_brightness(uint8_t level) {
    // CYD backlight is on/off only (GPIO 21). Ignore level — stay on.
    (void)level;
}

void display_hal_fill_screen(uint16_t color) {
    gfx->fillScreen(color);
}

void display_hal_draw_bitmap(int32_t x, int32_t y, int32_t w, int32_t h,
                             const uint16_t* pixels) {
    gfx->draw16bitRGBBitmap(x, y, (uint16_t*)pixels, w, h);
}

void display_hal_tick(void) {}

// ILI9341 via SPI has no alignment constraints.
void display_hal_round_area(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2) {
    (void)x1; (void)y1; (void)x2; (void)y2;
}
