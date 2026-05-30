#include "../../firmware/src/hal/display_hal.h"
#include "../../firmware/src/hal/board_caps.h"
#include <SDL2/SDL.h>
#include <cstdio>

static SDL_Window*   window   = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture*  texture  = nullptr;

void display_hal_init(void) {
    // SDL_Init() is called by main() before board_init() / display_hal_init().
}

void display_hal_begin(void) {
    const BoardCaps& c = board_caps();
    window = SDL_CreateWindow(
        "Clawdmeter Simulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        c.width, c.height,
        SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        c.width, c.height);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

// No-op: desktop brightness is not meaningful.
void display_hal_set_brightness(uint8_t level) { (void)level; }

void display_hal_fill_screen(uint16_t color) { (void)color; }

void display_hal_draw_bitmap(int32_t x, int32_t y, int32_t w, int32_t h,
                             const uint16_t* pixels) {
    SDL_Rect rect = { (int)x, (int)y, (int)w, (int)h };
    SDL_UpdateTexture(texture, &rect, pixels, (int)w * 2);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void display_hal_tick(void) {}

// SDL2 does not require even-aligned flush regions.
void display_hal_round_area(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2) {
    (void)x1; (void)y1; (void)x2; (void)y2;
}
