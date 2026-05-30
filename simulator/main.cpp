#include <SDL2/SDL.h>
#include <lvgl.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <thread>
#include <mutex>
#include <atomic>

#include "data.h"
#include "ui.h"
#include "splash.h"
#include "ble.h"
#include "idle.h"
#include "usage_rate.h"
#include "ApiPoller.h"

#include "hal/board_caps.h"
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "hal/input_hal.h"
#include "hal/power_hal.h"
#include "hal/imu_hal.h"

extern "C" void board_init(void);

// Shared state between poller thread and main loop.
static UsageData        g_latest   = {};
static std::mutex       g_mutex;
static std::atomic<bool> g_ready   {false};
static std::atomic<bool> g_running {true};

static void PollerThread() {
    ApiPoller poller;
    while (g_running) {
        UsageData d = {};
        if (poller.Poll(&d)) {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_latest = d;
            g_ready  = true;
        }
        // Sleep 60s in 1s increments so we can exit quickly on shutdown.
        for (int i = 0; i < 60 && g_running; i++)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

static uint32_t sim_tick_cb(void) {
    return (uint32_t)SDL_GetTicks();
}

static uint16_t* buf1 = nullptr;
static uint16_t* buf2 = nullptr;

static void my_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    display_hal_draw_bitmap(area->x1, area->y1, w, h, (uint16_t*)px_map);
    lv_display_flush_ready(disp);
}

static void my_touch_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    (void)indev;
    uint16_t x, y;
    bool pressed;
    touch_hal_read(&x, &y, &pressed);
    if (pressed) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    board_init();
    display_hal_init();
    display_hal_begin();
    idle_init();
    power_hal_init();
    imu_hal_init();
    touch_hal_init();

    const int W = board_caps().width;
    const int H = board_caps().height;

    lv_init();
    lv_tick_set_cb(sim_tick_cb);

    buf1 = (uint16_t*)malloc((size_t)W * 40 * 2);
    buf2 = (uint16_t*)malloc((size_t)W * 40 * 2);

    lv_display_t* disp = lv_display_create(W, H);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, my_flush_cb);
    lv_display_set_buffers(disp, buf1, buf2, (uint32_t)(W * 40 * 2),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_cb);

    input_hal_init();

    ui_init();
    ui_update_ble_status(BLE_STATE_CONNECTED,
                         ble_get_device_name(),
                         ble_get_mac_address());
    ui_update_battery(power_hal_battery_pct(), power_hal_is_charging());
    ui_show_screen(SCREEN_USAGE);

    // Start background API poller — first data arrives in ~2-3 seconds.
    std::thread poller_thread(PollerThread);

    printf("Simulator ready. F1=PRIMARY, F2=SECONDARY, F3=PWR, Esc=quit\n");
    printf("Polling Anthropic API — data will appear in a few seconds...\n");

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            if (ev.type == SDL_KEYDOWN &&
                ev.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Apply fresh data from poller thread if available.
        if (g_ready.exchange(false)) {
            UsageData d;
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                d = g_latest;
            }
            usage_rate_sample(d.session_pct);
            ui_update(&d);
        }

        idle_tick();
        lv_timer_handler();
        splash_tick();

        if (power_hal_pwr_pressed()) {
            if (ui_get_current_screen() == SCREEN_SPLASH) splash_next();
            else                                           ui_cycle_screen();
        }

        SDL_Delay(5);
    }

    g_running = false;
    poller_thread.join();

    free(buf1);
    free(buf2);
    SDL_Quit();
    return 0;
}
