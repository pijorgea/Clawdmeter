#pragma once
#include <stdint.h>
#include <stdbool.h>

// Persistent settings stored in ESP32 NVS (Non-Volatile Storage).
// Call settings_load() once at boot. Writes are immediate (Preferences library
// flushes on each put). All getters return empty string when no value is stored.

void        settings_load(void);

const char* settings_wifi_ssid(void);
const char* settings_wifi_pass(void);
const char* settings_api_key(void);

void        settings_set_wifi(const char* ssid, const char* pass);
void        settings_set_api_key(const char* key);

// True if all three fields are non-empty — device can attempt connection.
bool        settings_is_configured(void);
