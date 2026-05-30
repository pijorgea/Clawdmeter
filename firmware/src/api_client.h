#pragma once
#include "data.h"
#include <stdbool.h>

// Calls the Anthropic API directly over HTTPS and populates UsageData
// from the rate-limit response headers. Requires WiFi to be connected
// and a valid API key (sk-ant-...) from settings_api_key().
//
// Blocks for the duration of the HTTP request (~1-5s depending on network).
// Call from a background task or infrequent timer (every 60s).
//
// Returns true and fills *out on success; returns false on network error
// or missing API key.
bool api_client_poll(UsageData* out);
