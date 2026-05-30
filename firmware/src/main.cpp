#include <Arduino.h>
#include <lvgl.h>
#include <time.h>
#include <esp_heap_caps.h>

#include "data.h"
#include "ui.h"
#include "splash.h"
#include "settings.h"
#include "api_client.h"
#include "usage_rate.h"
#include "idle.h"
#include "idle_cfg.h"

#include "hal/board_caps.h"
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "hal/input_hal.h"
#include "hal/power_hal.h"
#include "hal/imu_hal.h"
#include "hal/wifi_hal.h"

static UsageData usage = {};

#ifdef BOARD_HAS_PSRAM
#define BUF_LINES 40
#define LV_BUF_CAPS (MALLOC_CAP_SPIRAM)
#else
#define BUF_LINES 20
#define LV_BUF_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#endif
static uint16_t* buf1 = nullptr;
static uint16_t* buf2 = nullptr;

static uint32_t my_tick(void) { return millis(); }

static void my_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    display_hal_draw_bitmap(area->x1, area->y1, w, h, (uint16_t*)px_map);
    lv_display_flush_ready(disp);
}

static void rounder_cb(lv_event_t* e) {
    lv_area_t* area = (lv_area_t*)lv_event_get_param(e);
    display_hal_round_area(&area->x1, &area->y1, &area->x2, &area->y2);
}

static void my_touch_cb(lv_indev_t* indev, lv_indev_data_t* data) {
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

extern "C" void board_init(void);

// ---- Serial command buffer ----
#define CMD_BUF_SIZE 64
static char cmd_buf[CMD_BUF_SIZE];
static int  cmd_pos = 0;

static void send_screenshot() {
#ifndef BOARD_HAS_PSRAM
    Serial.println("SCREENSHOT_UNSUPPORTED");
    return;
#else
    const uint32_t w = board_caps().width;
    const uint32_t h = board_caps().height;
    const uint32_t row_bytes = w * 2;
    const uint32_t buf_size  = row_bytes * h;
    uint8_t* sbuf = (uint8_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
    if (!sbuf) { Serial.println("SCREENSHOT_ERR"); return; }

    lv_draw_buf_t draw_buf;
    lv_draw_buf_init(&draw_buf, w, h, LV_COLOR_FORMAT_RGB565, row_bytes, sbuf, buf_size);
    if (lv_snapshot_take_to_draw_buf(lv_screen_active(), LV_COLOR_FORMAT_RGB565, &draw_buf)
            != LV_RESULT_OK) {
        heap_caps_free(sbuf);
        Serial.println("SCREENSHOT_ERR");
        return;
    }
    Serial.printf("SCREENSHOT_START %lu %lu %lu\n",
        (unsigned long)w, (unsigned long)h, (unsigned long)buf_size);
    Serial.flush();
    Serial.write(sbuf, buf_size);
    Serial.flush();
    Serial.println();
    Serial.println("SCREENSHOT_END");
    heap_caps_free(sbuf);
#endif
}

static void check_serial_cmd() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            cmd_buf[cmd_pos] = '\0';
            if (strcmp(cmd_buf, "screenshot") == 0) send_screenshot();
            cmd_pos = 0;
        } else if (cmd_pos < CMD_BUF_SIZE - 1) {
            cmd_buf[cmd_pos++] = c;
        }
    }
}

// ---- NTP sync (requires WiFi) ----
static void sync_ntp(void) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_t now = time(nullptr);
    int attempts = 0;
    while (now < 1700000000L && attempts < 20) {
        delay(500);
        now = time(nullptr);
        attempts++;
    }
    if (now >= 1700000000L)
        Serial.printf("[ntp] synced: %ld\n", (long)now);
    else
        Serial.println("[ntp] sync failed — reset times may be inaccurate");
}

// ---- Settings flow ----
static void handle_settings_save(void) {
    char ssid[65] = {}, pass[65] = {}, api_key[128] = {};
    ui_settings_get_values(ssid, sizeof(ssid), pass, sizeof(pass),
                           api_key, sizeof(api_key));

    if (ssid[0] == '\0' || api_key[0] == '\0') {
        ui_settings_set_status("SSID and API key are required.");
        return;
    }

    settings_set_wifi(ssid, pass);
    settings_set_api_key(api_key);
    ui_settings_set_status("Connecting to WiFi...");

    ui_update_wifi_status(WIFI_STATUS_CONNECTING, nullptr);
    if (!wifi_hal_connect(ssid, pass, 20000)) {
        ui_settings_set_status("WiFi connection failed. Check credentials.");
        ui_update_wifi_status(WIFI_STATUS_ERROR, nullptr);
        return;
    }

    ui_update_wifi_status(WIFI_STATUS_CONNECTED, wifi_hal_ip());
    ui_settings_set_status("Connected! Syncing time...");
    sync_ntp();

    ui_settings_set_status("Fetching usage data...");
    if (api_client_poll(&usage)) {
        usage_rate_sample(usage.session_pct);
        ui_update(&usage);
        ui_settings_set_status("Done.");
        delay(800);
        ui_show_screen(SCREEN_USAGE);
    } else {
        ui_settings_set_status("API call failed. Check API key.");
    }
}

// ---- API polling ----
static uint32_t last_poll_ms = 0;
#define POLL_INTERVAL_MS (60UL * 1000UL)

static void maybe_poll(void) {
    if (!wifi_hal_is_connected()) return;
    uint32_t now = millis();
    if (now - last_poll_ms < POLL_INTERVAL_MS) return;
    last_poll_ms = now;

    UsageData fresh = {};
    if (api_client_poll(&fresh)) {
        int g_before = usage_rate_group();
        usage_rate_sample(fresh.session_pct);
        int g_after = usage_rate_group();
        usage = fresh;
        ui_update(&usage);
        if (g_after != g_before && splash_is_active())
            splash_pick_for_current_rate();
    }
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("{\"ready\":true}");

    board_init();
    settings_load();

    display_hal_init();
    display_hal_begin();
    idle_init();
    power_hal_init();
    imu_hal_init();
    touch_hal_init();
    wifi_hal_init();

    const int W = board_caps().width;
    const int H = board_caps().height;

    lv_init();
    lv_tick_set_cb(my_tick);

    buf1 = (uint16_t*)heap_caps_malloc(W * BUF_LINES * 2, LV_BUF_CAPS);
    buf2 = (uint16_t*)heap_caps_malloc(W * BUF_LINES * 2, LV_BUF_CAPS);

    lv_display_t* disp = lv_display_create(W, H);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, my_flush_cb);
    lv_display_set_buffers(disp, buf1, buf2, W * BUF_LINES * 2,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_add_event_cb(disp, rounder_cb, LV_EVENT_INVALIDATE_AREA, NULL);

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_cb);

    input_hal_init();
    ui_init();
    ui_update_battery(power_hal_battery_pct(), power_hal_is_charging());

    if (settings_is_configured()) {
        // Try to connect in background — show splash while connecting.
        ui_show_screen(SCREEN_SPLASH);
        ui_update_wifi_status(WIFI_STATUS_CONNECTING, nullptr);
        lv_timer_handler();  // render one frame before blocking on WiFi

        if (wifi_hal_connect(settings_wifi_ssid(), settings_wifi_pass(), 20000)) {
            ui_update_wifi_status(WIFI_STATUS_CONNECTED, wifi_hal_ip());
            sync_ntp();
            if (api_client_poll(&usage)) {
                usage_rate_sample(usage.session_pct);
                ui_update(&usage);
            }
            last_poll_ms = millis();
        } else {
            ui_update_wifi_status(WIFI_STATUS_ERROR, nullptr);
        }
    } else {
        // First boot — no credentials yet.
        ui_show_screen(SCREEN_SETTINGS);
    }

    Serial.printf("Dashboard ready (%s, %dx%d)\n", board_caps().name, W, H);
}

void loop() {
    idle_tick();
    lv_timer_handler();
    ui_tick_anim();
    power_hal_tick();
    imu_hal_tick();
    splash_tick();
    wifi_hal_tick();

    if (!idle_is_asleep()) display_hal_tick();

    // Settings save button
    if (ui_settings_save_requested()) {
        handle_settings_save();
    }

    // Poll API periodically
    maybe_poll();

    // PWR button — cycle screens
    if (power_hal_pwr_pressed()) {
        if (!idle_consume_wake_press()) {
            if (ui_get_current_screen() == SCREEN_SPLASH) splash_next();
            else                                          ui_cycle_screen();
        }
    }

    // PRIMARY button (wake / HID placeholder)
    {
        static bool primary_was = false;
        static bool primary_wake_swallowed = false;
        bool primary_now = input_hal_is_held(INPUT_BTN_PRIMARY);
        if (primary_now != primary_was) {
            if (primary_now) {
                if (idle_consume_wake_press()) primary_wake_swallowed = true;
            } else {
                primary_wake_swallowed = false;
            }
            primary_was = primary_now;
        }
    }

    static int  last_pct      = -2;
    static bool last_charging = false;
    int  pct      = power_hal_battery_pct();
    bool charging = power_hal_is_charging();
    if (pct != last_pct || charging != last_charging) {
        last_pct = pct;
        last_charging = charging;
        ui_update_battery(pct, charging);
    }

    check_serial_cmd();
    delay(5);
}
