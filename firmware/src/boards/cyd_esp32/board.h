#pragma once

// ESP32-2432S028R (CYD — Cheap Yellow Display)
// Display: ILI9341, 320x240 landscape, VSPI
// Touch:   XPT2046 resistive (stubbed for now)
// WiFi:    ESP32 built-in
// No PSRAM, no IMU, no AXP PMU

#define LCD_MOSI  13
#define LCD_MISO  12
#define LCD_SCLK  14
#define LCD_CS    15
#define LCD_DC     2
#define LCD_RST   -1   // RST tied to EN on CYD
#define LCD_BL    21

#define LCD_WIDTH  320
#define LCD_HEIGHT 240

// Boot button (GPIO 0) — only physical button on the CYD
#define BTN_BOOT   0
