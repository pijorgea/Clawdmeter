#pragma once
#include <stdlib.h>
#include <stdint.h>

// ESP32 heap capability flags (ignored on desktop — all memory is equivalent).
#define MALLOC_CAP_SPIRAM    0x01
#define MALLOC_CAP_INTERNAL  0x02
#define MALLOC_CAP_8BIT      0x04
#define MALLOC_CAP_DMA       0x08

static inline void* heap_caps_malloc(size_t size, uint32_t caps) {
    (void)caps;
    return malloc(size);
}

static inline void heap_caps_free(void* ptr) {
    free(ptr);
}
