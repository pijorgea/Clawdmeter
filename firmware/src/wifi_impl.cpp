#include "hal/wifi_hal.h"
#include <Arduino.h>
#include <WiFi.h>

static char s_ip[24] = "";

void wifi_hal_init(void) {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
}

void wifi_hal_tick(void) {
    // setAutoReconnect handles reconnection; nothing manual needed per tick.
}

bool wifi_hal_connect(const char* ssid, const char* password, uint32_t timeout_ms) {
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start >= timeout_ms) {
            WiFi.disconnect();
            s_ip[0] = '\0';
            return false;
        }
        delay(200);
    }
    strncpy(s_ip, WiFi.localIP().toString().c_str(), sizeof(s_ip) - 1);
    return true;
}

void wifi_hal_disconnect(void) {
    WiFi.disconnect();
    s_ip[0] = '\0';
}

bool wifi_hal_is_connected(void) {
    return WiFi.status() == WL_CONNECTED;
}

const char* wifi_hal_ip(void) {
    return s_ip;
}

int wifi_hal_rssi(void) {
    return WiFi.RSSI();
}
