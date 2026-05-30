#include "../../hal/board_caps.h"
#include "board.h"

static const BoardCaps CYD_CAPS = {
    .name         = "CYD ESP32-2432S028R",
    .width        = LCD_WIDTH,
    .height       = LCD_HEIGHT,
    .button_count = 1,
    .has_rotation = false,
    .has_battery  = false,
    .has_imu      = false,
};

const BoardCaps& board_caps(void) {
    return CYD_CAPS;
}
