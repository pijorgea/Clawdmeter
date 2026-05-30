#include "settings.h"
#include <Preferences.h>
#include <cstring>

static Preferences prefs;

static char s_ssid[65]   = "";
static char s_pass[65]   = "";
static char s_api_key[128] = "";

void settings_load(void) {
    prefs.begin("clawdmeter", true);  // read-only
    strncpy(s_ssid,    prefs.getString("ssid",    "").c_str(), sizeof(s_ssid)    - 1);
    strncpy(s_pass,    prefs.getString("pass",    "").c_str(), sizeof(s_pass)    - 1);
    strncpy(s_api_key, prefs.getString("api_key", "").c_str(), sizeof(s_api_key) - 1);
    prefs.end();
}

const char* settings_wifi_ssid(void) { return s_ssid; }
const char* settings_wifi_pass(void) { return s_pass; }
const char* settings_api_key(void)   { return s_api_key; }

void settings_set_wifi(const char* ssid, const char* pass) {
    strncpy(s_ssid, ssid, sizeof(s_ssid) - 1);
    strncpy(s_pass, pass, sizeof(s_pass) - 1);
    prefs.begin("clawdmeter", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
}

void settings_set_api_key(const char* key) {
    strncpy(s_api_key, key, sizeof(s_api_key) - 1);
    prefs.begin("clawdmeter", false);
    prefs.putString("api_key", key);
    prefs.end();
}

bool settings_is_configured(void) {
    return s_ssid[0] != '\0' && s_pass[0] != '\0' && s_api_key[0] != '\0';
}
