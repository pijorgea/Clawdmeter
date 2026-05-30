#pragma once
#include <stdbool.h>

// WiFi connectivity abstraction.
// Each board implements this using the Arduino WiFi stack.
// Call wifi_hal_init() once at boot, then wifi_hal_connect() with credentials.
// wifi_hal_tick() must be called from loop() to maintain the connection.

void        wifi_hal_init(void);
void        wifi_hal_tick(void);

// Connect to an access point. Blocks until connected or timeout_ms expires.
// Returns true on success.
bool        wifi_hal_connect(const char* ssid, const char* password,
                             uint32_t timeout_ms = 15000);

void        wifi_hal_disconnect(void);
bool        wifi_hal_is_connected(void);

// Returns the assigned IP address string, or "" if not connected.
const char* wifi_hal_ip(void);

// Signal strength of the current AP (-dBm, lower = stronger).
int         wifi_hal_rssi(void);
