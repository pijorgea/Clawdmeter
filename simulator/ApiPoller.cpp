#include "ApiPoller.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

ApiPoller::ApiPoller() {
    m_credPath = FindCredentials();
    if (m_credPath.empty())
        printf("[poller] WARNING: credentials file not found\n");
    else
        printf("[poller] credentials: %s\n", m_credPath.c_str());
}

// Check Linux home first, then Windows home via WSL /mnt/c mount.
std::string ApiPoller::FindCredentials() {
    auto try_path = [](const std::string& p) -> std::string {
        std::ifstream f(p);
        return f.good() ? p : "";
    };

    const char* home = getenv("HOME");
    if (home) {
        std::string p = std::string(home) + "/.claude/.credentials.json";
        if (!try_path(p).empty()) return p;
    }

    const char* user = getenv("USER");
    if (user) {
        std::string p = std::string("/mnt/c/Users/") + user + "/.claude/.credentials.json";
        if (!try_path(p).empty()) return p;
    }

    // Fallback: try known Windows user path.
    std::string fallback = "/mnt/c/Users/Jorge/.claude/.credentials.json";
    if (!try_path(fallback).empty()) return fallback;

    return "";
}

// Extract "accessToken" value from the credentials JSON.
// Avoids pulling in a JSON library — the file is small and the field is unique.
std::string ApiPoller::ReadToken(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

    const char* key = "\"accessToken\":\"";
    auto pos = content.find(key);
    if (pos == std::string::npos) return "";
    pos += strlen(key);
    auto end = content.find('"', pos);
    if (end == std::string::npos) return "";
    return content.substr(pos, end - pos);
}

// Parse a float from a header line like "header-name: 0.45\r\n".
static bool ParseFloat(const std::string& line, const char* name, float& out) {
    std::string prefix = std::string(name) + ":";
    if (line.size() < prefix.size()) return false;
    // Case-insensitive compare of prefix
    for (size_t i = 0; i < prefix.size(); i++)
        if (tolower((unsigned char)line[i]) != tolower((unsigned char)prefix[i]))
            return false;
    try { out = std::stof(line.substr(prefix.size())); return true; }
    catch (...) { return false; }
}

static bool ParseLong(const std::string& line, const char* name, long& out) {
    std::string prefix = std::string(name) + ":";
    if (line.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++)
        if (tolower((unsigned char)line[i]) != tolower((unsigned char)prefix[i]))
            return false;
    try { out = std::stol(line.substr(prefix.size())); return true; }
    catch (...) { return false; }
}

static bool ParseStr(const std::string& line, const char* name, std::string& out) {
    std::string prefix = std::string(name) + ":";
    if (line.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); i++)
        if (tolower((unsigned char)line[i]) != tolower((unsigned char)prefix[i]))
            return false;
    out = line.substr(prefix.size());
    // Trim whitespace
    while (!out.empty() && (out.front() == ' ' || out.front() == '\t')) out.erase(0, 1);
    while (!out.empty() && (out.back() == ' ' || out.back() == '\r' || out.back() == '\n')) out.pop_back();
    return true;
}

bool ApiPoller::Poll(UsageData* out) {
    if (m_credPath.empty()) {
        printf("[poller] no credentials file — skipping poll\n");
        return false;
    }

    std::string token = ReadToken(m_credPath);
    if (token.empty()) {
        printf("[poller] could not read accessToken from credentials\n");
        return false;
    }

    // Build curl command — same flags as the daemon.
    // -s: silent, -D -: dump headers to stdout, -o /dev/null: discard body.
    std::string cmd =
        "curl -s -D - -o /dev/null "
        "--max-time 15 "
        "'https://api.anthropic.com/v1/messages' "
        "-H 'Authorization: Bearer " + token + "' "
        "-H 'anthropic-version: 2023-06-01' "
        "-H 'anthropic-beta: oauth-2025-04-20' "
        "-H 'Content-Type: application/json' "
        "-H 'User-Agent: claude-code/2.1.5' "
        "-d '{\"model\":\"claude-haiku-4-5-20251001\","
             "\"max_tokens\":1,"
             "\"messages\":[{\"role\":\"user\",\"content\":\"hi\"}]}' "
        "2>/dev/null";

    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp) {
        printf("[poller] popen failed\n");
        return false;
    }

    float  util5h = 0.0f;
    long   reset5h = 0;
    float  util7d = 0.0f;
    long   reset7d = 0;
    std::string status;

    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) {
        std::string line(buf);
        ParseFloat(line, "anthropic-ratelimit-unified-5h-utilization", util5h);
        ParseLong (line, "anthropic-ratelimit-unified-5h-reset",       reset5h);
        ParseFloat(line, "anthropic-ratelimit-unified-7d-utilization", util7d);
        ParseLong (line, "anthropic-ratelimit-unified-7d-reset",       reset7d);
        ParseStr  (line, "anthropic-ratelimit-unified-5h-status",      status);
    }
    pclose(fp);

    long now = (long)time(nullptr);

    out->session_pct        = util5h * 100.0f;
    out->session_reset_mins = reset5h > now ? (int)((reset5h - now) / 60) : 0;
    out->weekly_pct         = util7d * 100.0f;
    out->weekly_reset_mins  = reset7d > now ? (int)((reset7d - now) / 60) : 0;
    out->ok                 = true;
    out->valid              = true;

    if (status.empty()) status = "unknown";
    strncpy(out->status, status.c_str(), sizeof(out->status) - 1);
    out->status[sizeof(out->status) - 1] = '\0';

    printf("[poller] session=%.1f%% (reset %dmin)  weekly=%.1f%% (reset %dmin)  status=%s\n",
        out->session_pct, out->session_reset_mins,
        out->weekly_pct,  out->weekly_reset_mins,
        out->status);

    return true;
}
