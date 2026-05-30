#pragma once
#include <stdint.h>
#include <stdbool.h>

// Minimal HTTPS client abstraction for Anthropic API calls.
// Each board implements this using Arduino HTTPClient + WiFiClientSecure.
//
// Usage:
//   HttpRequest req;
//   http_hal_init_request(&req, "https://api.anthropic.com/v1/messages");
//   http_hal_add_header(&req, "Authorization", "Bearer sk-ant-...");
//   int status = http_hal_post(&req, body, body_len);
//   if (status == 200 || status == 400) {  // 400 = bad request but headers present
//       const char* val = http_hal_get_response_header(&req, "anthropic-ratelimit-...");
//   }
//   http_hal_free_request(&req);

struct HttpRequest;  // opaque, board-allocated

HttpRequest* http_hal_create(void);
void         http_hal_destroy(HttpRequest* req);

void         http_hal_set_url(HttpRequest* req, const char* url);
void         http_hal_add_header(HttpRequest* req, const char* name, const char* value);

// Performs POST. Returns HTTP status code, or <0 on network error.
int          http_hal_post(HttpRequest* req, const char* body, int body_len);

// Returns the value of a response header (case-insensitive), or "" if absent.
const char*  http_hal_get_response_header(HttpRequest* req, const char* name);
