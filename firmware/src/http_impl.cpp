#include "hal/http_hal.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <map>
#include <string>
#include <algorithm>

struct HttpRequest {
    std::string                        url;
    std::map<std::string, std::string> req_headers;
    std::map<std::string, std::string> res_headers;
};

HttpRequest* http_hal_create(void) {
    return new HttpRequest();
}

void http_hal_destroy(HttpRequest* req) {
    delete req;
}

void http_hal_set_url(HttpRequest* req, const char* url) {
    req->url = url;
}

void http_hal_add_header(HttpRequest* req, const char* name, const char* value) {
    // Store lowercase name for case-insensitive lookup later.
    std::string key(name);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    req->req_headers[key] = value;
}

int http_hal_post(HttpRequest* req, const char* body, int body_len) {
    req->res_headers.clear();

    WiFiClientSecure client;
    client.setInsecure();  // skip cert verification — Anthropic uses public CA

    HTTPClient http;
    if (!http.begin(client, req->url.c_str())) return -1;

    // Set request headers
    for (auto& kv : req->req_headers)
        http.addHeader(kv.first.c_str(), kv.second.c_str());

    // Tell HTTPClient which response headers to collect
    static const char* WANTED[] = {
        "anthropic-ratelimit-unified-5h-utilization",
        "anthropic-ratelimit-unified-5h-reset",
        "anthropic-ratelimit-unified-7d-utilization",
        "anthropic-ratelimit-unified-7d-reset",
        "anthropic-ratelimit-unified-5h-status",
    };
    http.collectHeaders(WANTED, 5);

    int status = http.POST((uint8_t*)body, body_len);

    if (status > 0) {
        for (auto& name : WANTED) {
            String val = http.header(name);
            if (val.length() > 0) {
                std::string key(name);
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                req->res_headers[key] = val.c_str();
            }
        }
    }

    http.end();
    return status;
}

const char* http_hal_get_response_header(HttpRequest* req, const char* name) {
    std::string key(name);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    auto it = req->res_headers.find(key);
    if (it == req->res_headers.end()) return "";
    return it->second.c_str();
}
