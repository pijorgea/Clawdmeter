#include "ble.h"

static ble_state_t g_state = BLE_STATE_CONNECTED;

void         ble_init(void)              {}
void         ble_tick(void)              {}
ble_state_t  ble_get_state(void)         { return g_state; }
const char*  ble_get_device_name(void)   { return "Clawdmeter Sim"; }
const char*  ble_get_mac_address(void)   { return "AA:BB:CC:DD:EE:FF"; }
void         ble_clear_bonds(void)       {}
bool         ble_has_data(void)          { return false; }
const char*  ble_get_data(void)          { return "{}"; }
void         ble_send_ack(void)          {}
void         ble_send_nack(void)         {}
void         ble_request_refresh(void)   {}
void         ble_keyboard_press(uint8_t key, uint8_t modifier) { (void)key; (void)modifier; }
void         ble_keyboard_release(void)  {}
