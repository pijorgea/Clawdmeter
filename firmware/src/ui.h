#pragma once
#include "data.h"

// WiFi connection status passed to ui_update_wifi_status().
enum wifi_status_t {
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_ERROR,
};

enum screen_t {
    SCREEN_SPLASH,
    SCREEN_USAGE,
    SCREEN_SETTINGS,
    SCREEN_COUNT,
};

void ui_init(void);
void ui_update(const UsageData* data);
void ui_tick_anim(void);
void ui_show_screen(screen_t screen);
void ui_cycle_screen(void);
screen_t ui_get_current_screen(void);

// WiFi + API status shown on the usage screen header and settings screen.
void ui_update_wifi_status(wifi_status_t status, const char* ip);

// Battery indicator (unchanged).
void ui_update_battery(int percent, bool charging);

// Settings screen: read back what the user typed before saving.
void ui_settings_get_values(char* ssid, int ssid_len,
                             char* pass, int pass_len,
                             char* api_key, int key_len);

// Settings screen: prefill fields with stored values.
void ui_settings_set_values(const char* ssid, const char* pass,
                             const char* api_key);

// Settings screen: update the status line shown below the fields.
void ui_settings_set_status(const char* msg);

// Returns true once when the user pressed "Save & Connect", then resets.
bool ui_settings_save_requested(void);
