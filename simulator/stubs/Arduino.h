#pragma once
#include <stdint.h>
#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <cstring>
#include <unistd.h>

static inline uint32_t millis(void) {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return (uint32_t)duration_cast<milliseconds>(steady_clock::now() - start).count();
}

static inline void delay(uint32_t ms) {
    usleep((useconds_t)ms * 1000);
}

// Minimal Serial stub — routes to stdout.
struct SerialStub {
    void begin(int baud) { (void)baud; }
    void println(const char* s) { printf("%s\n", s); }
    template<typename T>
    void println(T v) { printf("%d\n", (int)v); }
    void printf(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
    void write(const uint8_t* buf, size_t n) { fwrite(buf, 1, n, stdout); }
    void flush(void) { fflush(stdout); }
    int  available(void) { return 0; }
    int  read(void) { return -1; }
};
static SerialStub Serial;
