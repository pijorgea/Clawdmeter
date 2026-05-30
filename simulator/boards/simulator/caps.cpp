#include "../../firmware/src/hal/board_caps.h"

static const BoardCaps SIM_CAPS = {
    .name         = "Simulator",
    .width        = 480,
    .height       = 480,
    .button_count = 2,
    .has_rotation = false,
    .has_battery  = true,
    .has_imu      = false,
};

const BoardCaps& board_caps(void) {
    return SIM_CAPS;
}
