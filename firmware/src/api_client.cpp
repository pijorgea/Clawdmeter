#include "api_client.h"
#include "settings.h"
#include "hal/http_hal.h"
#include <Arduino.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>

static const char* API_URL =
    "https://api.anthropic.com/v1/messages";

static const char* API_BODY =
    "{\"model\":\"claude-haiku-4-5-20251001\","
    "\"max_tokens\":1,"
    "\"messages\":[{\"role\":\"user\",\"content\":\"hi\"}]}";

bool api_client_poll(UsageData* out) {
    const char* key = settings_api_key();
    if (!key || key[0] == '\0') {
        Serial.println("[api] no API key configured");
        return false;
    }

    char auth[148];
    snprintf(auth, sizeof(auth), "Bearer %s", key);

    HttpRequest* req = http_hal_create();
    http_hal_set_url(req, API_URL);
    http_hal_add_header(req, "Authorization",    auth);
    http_hal_add_header(req, "anthropic-version", "2023-06-01");
    http_hal_add_header(req, "anthropic-beta",    "oauth-2025-04-20");
    http_hal_add_header(req, "Content-Type",      "application/json");
    http_hal_add_header(req, "User-Agent",        "clawdmeter/2.0");

    int status = http_hal_post(req, API_BODY, strlen(API_BODY));

    if (status <= 0) {
        Serial.printf("[api] HTTP error: %d\n", status);
        http_hal_destroy(req);
        return false;
    }

    // Read rate-limit headers (present even on 4xx responses).
    float util5h  = atof(http_hal_get_response_header(req,
        "anthropic-ratelimit-unified-5h-utilization"));
    long  reset5h = atol(http_hal_get_response_header(req,
        "anthropic-ratelimit-unified-5h-reset"));
    float util7d  = atof(http_hal_get_response_header(req,
        "anthropic-ratelimit-unified-7d-utilization"));
    long  reset7d = atol(http_hal_get_response_header(req,
        "anthropic-ratelimit-unified-7d-reset"));
    const char* st = http_hal_get_response_header(req,
        "anthropic-ratelimit-unified-5h-status");

    http_hal_destroy(req);

    long now = (long)time(nullptr);

    out->session_pct        = util5h * 100.0f;
    out->session_reset_mins = (reset5h > now) ? (int)((reset5h - now) / 60) : 0;
    out->weekly_pct         = util7d * 100.0f;
    out->weekly_reset_mins  = (reset7d > now) ? (int)((reset7d - now) / 60) : 0;
    out->ok                 = true;
    out->valid              = true;

    const char* status_str = (st && st[0]) ? st : "unknown";
    strncpy(out->status, status_str, sizeof(out->status) - 1);
    out->status[sizeof(out->status) - 1] = '\0';

    Serial.printf("[api] session=%.1f%% (reset %dmin)  weekly=%.1f%% (reset %dmin)  status=%s\n",
        out->session_pct, out->session_reset_mins,
        out->weekly_pct,  out->weekly_reset_mins,
        out->status);

    return true;
}
