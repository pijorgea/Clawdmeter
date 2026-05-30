#include "../../hal/power_hal.h"

// CYD has no PMU and no battery connector — all stubs.
void power_hal_init(void)    {}
void power_hal_tick(void)    {}
int  power_hal_battery_pct(void)  { return -1; }
bool power_hal_is_charging(void)  { return false; }
bool power_hal_is_vbus_in(void)   { return true; }
bool power_hal_pwr_pressed(void)  { return false; }
