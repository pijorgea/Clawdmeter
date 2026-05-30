#pragma once
#include <string>
#include "data.h"

// Polls the Anthropic API directly — same logic as the bash daemon but in C++.
// Reads the OAuth token from ~/.claude/.credentials.json (checks Linux home
// first, then the Windows home via /mnt/c/Users/<user>/).
// Entry point: Poll() — blocks for the duration of the HTTP request (~1-3s).
// Intended to be called from a background thread.
class ApiPoller {
public:
    ApiPoller();

    // Makes one API call. Returns true and populates *out on success.
    bool Poll(UsageData* out);

private:
    std::string FindCredentials();
    std::string ReadToken(const std::string& path);

    std::string m_credPath;
};
