#include "ui.h"
#include "splash.h"
#include <lvgl.h>
#include "logo.h"
#include "icons.h"
#include "hal/board_caps.h"
#include "theme.h"

LV_FONT_DECLARE(font_tiempos_56);
LV_FONT_DECLARE(font_tiempos_34);
LV_FONT_DECLARE(font_styrene_48);
LV_FONT_DECLARE(font_styrene_28);
LV_FONT_DECLARE(font_styrene_24);
LV_FONT_DECLARE(font_styrene_20);
LV_FONT_DECLARE(font_styrene_16);
LV_FONT_DECLARE(font_styrene_14);
LV_FONT_DECLARE(font_mono_32);

#define COL_BG        THEME_BG
#define COL_PANEL     THEME_PANEL
#define COL_TEXT      THEME_TEXT
#define COL_DIM       THEME_DIM
#define COL_ACCENT    THEME_ACCENT
#define COL_GREEN     THEME_GREEN
#define COL_AMBER     THEME_AMBER
#define COL_RED       THEME_RED
#define COL_BAR_BG    THEME_BAR_BG

struct Layout {
    int16_t scr_w, scr_h;
    int16_t margin;
    int16_t title_y;
    int16_t content_y;
    int16_t content_w;
    int16_t usage_panel_h;
    int16_t usage_panel_gap;
    int16_t usage_bar_y;
    int16_t usage_reset_y;
    const lv_font_t* title_font;
    const lv_font_t* field_font;
    const lv_font_t* small_font;
};
static Layout L = {};

static void compute_layout(const BoardCaps& c) {
    L.scr_w = c.width;
    L.scr_h = c.height;
    L.margin = 20;
    L.title_y = 30;
    L.content_w = L.scr_w - 2 * L.margin;

    if (c.height >= 460) {
        L.content_y      = 100;
        L.usage_panel_h  = 150;
        L.usage_panel_gap = 16;
        L.usage_bar_y    = 56;
        L.usage_reset_y  = 94;
        L.title_font     = &font_tiempos_56;
        L.field_font     = &font_styrene_28;
        L.small_font     = &font_styrene_20;
    } else {
        L.content_y      = 85;
        L.usage_panel_h  = 130;
        L.usage_panel_gap = 12;
        L.usage_bar_y    = 48;
        L.usage_reset_y  = 78;
        L.title_font     = &font_tiempos_34;
        L.field_font     = &font_styrene_20;
        L.small_font     = &font_styrene_16;
    }
}

// ---- Usage screen ----
static lv_obj_t* usage_container;
static lv_obj_t* lbl_title;
static lv_obj_t* lbl_wifi_status;
static lv_obj_t* bar_session;
static lv_obj_t* lbl_session_pct;
static lv_obj_t* lbl_session_label;
static lv_obj_t* lbl_session_reset;
static lv_obj_t* bar_weekly;
static lv_obj_t* lbl_weekly_pct;
static lv_obj_t* lbl_weekly_label;
static lv_obj_t* lbl_weekly_reset;
static lv_obj_t* lbl_anim;

// ---- Settings screen ----
static lv_obj_t* settings_container;
static lv_obj_t* ta_ssid;
static lv_obj_t* ta_pass;
static lv_obj_t* ta_api_key;
static lv_obj_t* lbl_settings_status;
static lv_obj_t* kb;
static bool      settings_save_requested = false;

// ---- Shared ----
static lv_obj_t*     battery_img;
static lv_obj_t*     logo_img;
static lv_image_dsc_t battery_dscs[5];
static lv_image_dsc_t logo_dsc;
static screen_t      current_screen = SCREEN_USAGE;

// ---- Animation ----
static uint32_t anim_last_ms = 0;
static uint8_t  anim_spinner_idx = 0;
static uint8_t  anim_phase = 0;
static uint8_t  anim_msg_idx = 0;
static uint32_t anim_msg_start = 0;
#define ANIM_MSG_MS 4000

static const char* const spinner_frames[] = {
    "\xC2\xB7", "\xE2\x9C\xBB", "\xE2\x9C\xBD",
    "\xE2\x9C\xB6", "\xE2\x9C\xB3", "\xE2\x9C\xA2",
};
#define SPINNER_COUNT 6
#define SPINNER_PHASES (2 * (SPINNER_COUNT - 1))

static const uint16_t spinner_ms[SPINNER_COUNT] = {
    260, 130, 130, 130, 130, 260,
};

static const char* const anim_messages[] = {
    "Accomplishing", "Elucidating", "Perusing",
    "Actioning", "Enchanting", "Philosophising",
    "Actualizing", "Envisioning", "Pondering",
    "Baking", "Finagling", "Pontificating",
    "Booping", "Flibbertigibbeting", "Processing",
    "Brewing", "Forging", "Puttering",
    "Calculating", "Forming", "Puzzling",
    "Cerebrating", "Frolicking", "Reticulating",
    "Channelling", "Generating", "Ruminating",
    "Churning", "Germinating", "Scheming",
    "Clauding", "Hatching", "Schlepping",
    "Coalescing", "Herding", "Shimmying",
    "Cogitating", "Honking", "Shucking",
    "Combobulating", "Hustling", "Simmering",
    "Computing", "Ideating", "Smooshing",
    "Concocting", "Imagining", "Spelunking",
    "Conjuring", "Incubating", "Spinning",
    "Considering", "Inferring", "Stewing",
    "Contemplating", "Jiving", "Sussing",
    "Cooking", "Manifesting", "Synthesizing",
    "Crafting", "Marinating", "Thinking",
    "Creating", "Meandering", "Tinkering",
    "Crunching", "Moseying", "Transmuting",
    "Deciphering", "Mulling", "Unfurling",
    "Deliberating", "Mustering", "Unravelling",
    "Determining", "Musing", "Vibing",
    "Divining", "Noodling", "Wandering",
    "Doing", "Percolating", "Whirring",
    "Effecting", "Wibbling", "Wizarding",
    "Working", "Wrangling",
};
#define ANIM_MSG_COUNT (sizeof(anim_messages) / sizeof(anim_messages[0]))

static lv_color_t pct_color(float pct) {
    if (pct >= 80.0f) return COL_RED;
    if (pct >= 50.0f) return COL_AMBER;
    return COL_GREEN;
}

static void format_reset_time(int mins, char* buf, size_t len) {
    if (mins < 0)          snprintf(buf, len, "---");
    else if (mins < 60)    snprintf(buf, len, "Resets in %dm", mins);
    else if (mins < 1440)  snprintf(buf, len, "Resets in %dh %dm", mins / 60, mins % 60);
    else                   snprintf(buf, len, "Resets in %dd %dh", mins / 1440, (mins % 1440) / 60);
}

static lv_obj_t* make_panel(lv_obj_t* parent, int x, int y, int w, int h) {
    lv_obj_t* p = lv_obj_create(parent);
    lv_obj_set_pos(p, x, y);
    lv_obj_set_size(p, w, h);
    lv_obj_set_style_bg_color(p, COL_PANEL, 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(p, 8, 0);
    lv_obj_set_style_border_width(p, 0, 0);
    lv_obj_set_style_pad_left(p, 16, 0);
    lv_obj_set_style_pad_right(p, 16, 0);
    lv_obj_set_style_pad_top(p, 12, 0);
    lv_obj_set_style_pad_bottom(p, 12, 0);
    lv_obj_clear_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(p, LV_OBJ_FLAG_EVENT_BUBBLE);
    return p;
}

static lv_obj_t* make_bar(lv_obj_t* parent, int x, int y, int w, int h) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_size(bar, w, h);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, COL_BAR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, COL_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 6, LV_PART_INDICATOR);
    return bar;
}

static void init_icon_dsc(lv_image_dsc_t* dsc, int w, int h, const uint16_t* data) {
    dsc->header.w = w;  dsc->header.h = h;
    dsc->header.cf = LV_COLOR_FORMAT_RGB565;
    dsc->header.stride = w * 2;
    dsc->data = (const uint8_t*)data;
    dsc->data_size = w * h * 2;
}

static void init_icon_dsc_rgb565a8(lv_image_dsc_t* dsc, int w, int h, const uint8_t* data) {
    dsc->header.w = w;  dsc->header.h = h;
    dsc->header.cf = LV_COLOR_FORMAT_RGB565A8;
    dsc->header.stride = w * 2;
    dsc->data = data;
    dsc->data_size = w * h * 3;
}

static lv_obj_t* make_pill(lv_obj_t* parent, const char* text) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, &font_styrene_28, 0);
    lv_obj_set_style_text_color(lbl, COL_TEXT, 0);
    lv_obj_set_style_bg_color(lbl, COL_BAR_BG, 0);
    lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(lbl, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_left(lbl, 18, 0);
    lv_obj_set_style_pad_right(lbl, 18, 0);
    lv_obj_set_style_pad_top(lbl, 6, 0);
    lv_obj_set_style_pad_bottom(lbl, 6, 0);
    return lbl;
}

static void init_battery_icons(void) {
    init_icon_dsc_rgb565a8(&battery_dscs[0], ICON_BATTERY_W, ICON_BATTERY_H, icon_battery_data);
    init_icon_dsc_rgb565a8(&battery_dscs[1], ICON_BATTERY_LOW_W, ICON_BATTERY_LOW_H, icon_battery_low_data);
    init_icon_dsc_rgb565a8(&battery_dscs[2], ICON_BATTERY_MEDIUM_W, ICON_BATTERY_MEDIUM_H, icon_battery_medium_data);
    init_icon_dsc_rgb565a8(&battery_dscs[3], ICON_BATTERY_FULL_W, ICON_BATTERY_FULL_H, icon_battery_full_data);
    init_icon_dsc_rgb565a8(&battery_dscs[4], ICON_BATTERY_CHARGING_W, ICON_BATTERY_CHARGING_H, icon_battery_charging_data);
}

// ======== Usage Screen ========

static void make_usage_panel(lv_obj_t* parent, int y, const char* pill_text,
                             lv_obj_t** out_pct, lv_obj_t** out_pill,
                             lv_obj_t** out_bar, lv_obj_t** out_reset) {
    lv_obj_t* panel = make_panel(parent, L.margin, y, L.content_w, L.usage_panel_h);

    *out_pct = lv_label_create(panel);
    lv_label_set_text(*out_pct, "---%");
    lv_obj_set_style_text_font(*out_pct, &font_styrene_48, 0);
    lv_obj_set_style_text_color(*out_pct, COL_TEXT, 0);
    lv_obj_set_pos(*out_pct, 0, 0);

    *out_pill = make_pill(panel, pill_text);
    lv_obj_align(*out_pill, LV_ALIGN_TOP_RIGHT, 0, 1);

    *out_bar = make_bar(panel, 0, L.usage_bar_y, L.content_w - 32, 24);

    *out_reset = lv_label_create(panel);
    lv_label_set_text(*out_reset, "---");
    lv_obj_set_style_text_font(*out_reset, &font_styrene_28, 0);
    lv_obj_set_style_text_color(*out_reset, COL_DIM, 0);
    lv_obj_set_pos(*out_reset, 0, L.usage_reset_y);
}

static void init_usage_screen(lv_obj_t* scr) {
    usage_container = lv_obj_create(scr);
    lv_obj_set_size(usage_container, L.scr_w, L.scr_h);
    lv_obj_set_pos(usage_container, 0, 0);
    lv_obj_set_style_bg_opa(usage_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(usage_container, 0, 0);
    lv_obj_set_style_pad_all(usage_container, 0, 0);
    lv_obj_clear_flag(usage_container, LV_OBJ_FLAG_SCROLLABLE);

    lbl_title = lv_label_create(usage_container);
    lv_label_set_text(lbl_title, "Usage");
    lv_obj_set_style_text_font(lbl_title, L.title_font, 0);
    lv_obj_set_style_text_color(lbl_title, COL_TEXT, 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 16, L.title_y);

    // WiFi status — small line below the title
    lbl_wifi_status = lv_label_create(usage_container);
    lv_label_set_text(lbl_wifi_status, "WiFi: connecting...");
    lv_obj_set_style_text_font(lbl_wifi_status, L.small_font, 0);
    lv_obj_set_style_text_color(lbl_wifi_status, COL_DIM, 0);
    lv_obj_align(lbl_wifi_status, LV_ALIGN_TOP_MID, 16, L.title_y + 50);

    make_usage_panel(usage_container, L.content_y, "Current",
                     &lbl_session_pct, &lbl_session_label,
                     &bar_session, &lbl_session_reset);
    make_usage_panel(usage_container,
                     L.content_y + L.usage_panel_h + L.usage_panel_gap, "Weekly",
                     &lbl_weekly_pct, &lbl_weekly_label,
                     &bar_weekly, &lbl_weekly_reset);

    lbl_anim = lv_label_create(usage_container);
    lv_label_set_text(lbl_anim, "");
    lv_obj_set_style_text_font(lbl_anim, &font_mono_32, 0);
    lv_obj_set_style_text_color(lbl_anim, COL_ACCENT, 0);
    lv_obj_align(lbl_anim, LV_ALIGN_BOTTOM_MID, 0, -15);
}

// ======== Settings Screen ========

static void kb_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_height(settings_container, L.scr_h);
    }
}

static void ta_focus_cb(lv_event_t* e) {
    lv_obj_t* ta = (lv_obj_t*)lv_event_get_target(e);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    // Shrink the scrollable area so the keyboard doesn't overlap fields.
    lv_obj_set_height(settings_container, L.scr_h - lv_obj_get_height(kb));
    lv_obj_scroll_to_view(ta, LV_ANIM_ON);
}

static void save_btn_cb(lv_event_t* e) {
    (void)e;
    settings_save_requested = true;
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t* make_field(lv_obj_t* parent, const char* label_text,
                             bool password, int y) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, label_text);
    lv_obj_set_style_text_font(lbl, L.small_font, 0);
    lv_obj_set_style_text_color(lbl, COL_DIM, 0);
    lv_obj_set_pos(lbl, 0, y);

    int lbl_h = lv_obj_get_height(lbl);
    lv_obj_t* ta = lv_textarea_create(parent);
    lv_obj_set_size(ta, L.content_w, 52);
    lv_obj_set_pos(ta, 0, y + lbl_h + 4);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_password_mode(ta, password);
    lv_obj_set_style_text_font(ta, L.field_font, 0);
    lv_obj_set_style_text_color(ta, COL_TEXT, 0);
    lv_obj_set_style_bg_color(ta, COL_PANEL, 0);
    lv_obj_set_style_bg_opa(ta, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ta, COL_ACCENT, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(ta, 2, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(ta, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(ta, COL_BAR_BG, LV_PART_MAIN);
    lv_obj_add_event_cb(ta, ta_focus_cb, LV_EVENT_FOCUSED, NULL);
    return ta;
}

static void init_settings_screen(lv_obj_t* scr) {
    settings_container = lv_obj_create(scr);
    lv_obj_set_size(settings_container, L.scr_w, L.scr_h);
    lv_obj_set_pos(settings_container, 0, 0);
    lv_obj_set_style_bg_color(settings_container, COL_BG, 0);
    lv_obj_set_style_bg_opa(settings_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(settings_container, 0, 0);
    lv_obj_set_style_pad_hor(settings_container, L.margin, 0);
    lv_obj_set_style_pad_ver(settings_container, L.margin, 0);
    lv_obj_set_flex_flow(settings_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_container, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* lbl_title_s = lv_label_create(settings_container);
    lv_label_set_text(lbl_title_s, "Settings");
    lv_obj_set_style_text_font(lbl_title_s, L.title_font, 0);
    lv_obj_set_style_text_color(lbl_title_s, COL_TEXT, 0);
    lv_obj_set_style_pad_bottom(lbl_title_s, 16, 0);

    // Fields — using absolute positioning inside a plain container
    lv_obj_t* form = lv_obj_create(settings_container);
    lv_obj_set_size(form, L.content_w, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(form, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(form, 0, 0);
    lv_obj_set_style_pad_all(form, 0, 0);

    ta_ssid    = make_field(form, "WiFi SSID",     false, 0);
    ta_pass    = make_field(form, "WiFi Password", true,  80);
    ta_api_key = make_field(form, "API Key (sk-ant-...)", false, 160);

    lbl_settings_status = lv_label_create(settings_container);
    lv_label_set_text(lbl_settings_status, "");
    lv_obj_set_style_text_font(lbl_settings_status, L.small_font, 0);
    lv_obj_set_style_text_color(lbl_settings_status, COL_DIM, 0);
    lv_obj_set_style_pad_top(lbl_settings_status, 12, 0);

    lv_obj_t* save_btn = lv_btn_create(settings_container);
    lv_obj_set_size(save_btn, L.content_w, 52);
    lv_obj_set_style_bg_color(save_btn, COL_ACCENT, 0);
    lv_obj_set_style_bg_opa(save_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(save_btn, 8, 0);
    lv_obj_set_style_pad_top(save_btn, 16, 0);
    lv_obj_add_event_cb(save_btn, save_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* save_lbl = lv_label_create(save_btn);
    lv_label_set_text(save_lbl, "Save & Connect");
    lv_obj_set_style_text_font(save_lbl, L.field_font, 0);
    lv_obj_set_style_text_color(save_lbl, COL_TEXT, 0);
    lv_obj_center(save_lbl);

    // Keyboard — shared between all text areas, hidden by default
    kb = lv_keyboard_create(scr);
    lv_obj_set_size(kb, L.scr_w, L.scr_h * 40 / 100);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_CANCEL, NULL);

    lv_obj_add_flag(settings_container, LV_OBJ_FLAG_HIDDEN);
}

// ======== Public API ========

void ui_init(void) {
    compute_layout(board_caps());

    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, COL_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    init_icon_dsc_rgb565a8(&logo_dsc, LOGO_WIDTH, LOGO_HEIGHT, logo_data);
    init_battery_icons();

    init_usage_screen(scr);
    init_settings_screen(scr);
    splash_init(scr);

    logo_img = lv_image_create(scr);
    lv_image_set_src(logo_img, &logo_dsc);
    lv_obj_set_pos(logo_img, L.margin, L.title_y - 10);

    battery_img = lv_image_create(scr);
    lv_image_set_src(battery_img, &battery_dscs[0]);
    lv_obj_set_pos(battery_img, L.scr_w - 48 - L.margin, L.title_y);
}

void ui_update(const UsageData* data) {
    if (!data->valid) return;

    int s_pct = (int)(data->session_pct + 0.5f);
    lv_label_set_text_fmt(lbl_session_pct, "%d%%", s_pct);
    lv_bar_set_value(bar_session, s_pct, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar_session, pct_color(data->session_pct), LV_PART_INDICATOR);

    char buf[48];
    format_reset_time(data->session_reset_mins, buf, sizeof(buf));
    lv_label_set_text(lbl_session_reset, buf);

    int w_pct = (int)(data->weekly_pct + 0.5f);
    lv_label_set_text_fmt(lbl_weekly_pct, "%d%%", w_pct);
    lv_bar_set_value(bar_weekly, w_pct, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar_weekly, pct_color(data->weekly_pct), LV_PART_INDICATOR);

    format_reset_time(data->weekly_reset_mins, buf, sizeof(buf));
    lv_label_set_text(lbl_weekly_reset, buf);
}

void ui_tick_anim(void) {
    if (current_screen != SCREEN_USAGE) return;

    uint32_t now = lv_tick_get();

    if (now - anim_msg_start >= ANIM_MSG_MS) {
        anim_msg_idx = (anim_msg_idx + 1) % ANIM_MSG_COUNT;
        anim_msg_start = now;
    }

    if (now - anim_last_ms >= spinner_ms[anim_spinner_idx]) {
        anim_last_ms = now;
        anim_phase = (anim_phase + 1) % SPINNER_PHASES;
        anim_spinner_idx = (anim_phase < SPINNER_COUNT) ? anim_phase
                                                        : (SPINNER_PHASES - anim_phase);
        static char buf[80];
        snprintf(buf, sizeof(buf), "%s %s\xE2\x80\xA6",
                 spinner_frames[anim_spinner_idx],
                 anim_messages[anim_msg_idx]);
        lv_label_set_text(lbl_anim, buf);
    }
}

static void apply_chrome_visibility(void) {
    bool show_chrome = (current_screen != SCREEN_SPLASH &&
                        current_screen != SCREEN_SETTINGS);
    if (logo_img) {
        if (show_chrome) lv_obj_clear_flag(logo_img, LV_OBJ_FLAG_HIDDEN);
        else             lv_obj_add_flag(logo_img, LV_OBJ_FLAG_HIDDEN);
    }
    if (battery_img) {
        if (show_chrome) lv_obj_clear_flag(battery_img, LV_OBJ_FLAG_HIDDEN);
        else             lv_obj_add_flag(battery_img, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_show_screen(screen_t screen) {
    lv_obj_add_flag(usage_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(settings_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    splash_hide();

    switch (screen) {
    case SCREEN_SPLASH:   splash_show(); break;
    case SCREEN_USAGE:    lv_obj_clear_flag(usage_container, LV_OBJ_FLAG_HIDDEN); break;
    case SCREEN_SETTINGS: lv_obj_clear_flag(settings_container, LV_OBJ_FLAG_HIDDEN); break;
    default: break;
    }

    current_screen = screen;
    apply_chrome_visibility();
}

void ui_cycle_screen(void) {
    screen_t next;
    switch (current_screen) {
    case SCREEN_USAGE:    next = SCREEN_SETTINGS; break;
    case SCREEN_SETTINGS: next = SCREEN_SPLASH;   break;
    case SCREEN_SPLASH:   next = SCREEN_USAGE;    break;
    default:              next = SCREEN_USAGE;    break;
    }
    ui_show_screen(next);
}

screen_t ui_get_current_screen(void) { return current_screen; }

void ui_update_wifi_status(wifi_status_t status, const char* ip) {
    if (!lbl_wifi_status) return;
    static char buf[48];
    switch (status) {
    case WIFI_STATUS_CONNECTED:
        snprintf(buf, sizeof(buf), "WiFi: %s", ip ? ip : "connected");
        lv_obj_set_style_text_color(lbl_wifi_status, COL_GREEN, 0);
        break;
    case WIFI_STATUS_CONNECTING:
        snprintf(buf, sizeof(buf), "WiFi: connecting...");
        lv_obj_set_style_text_color(lbl_wifi_status, COL_AMBER, 0);
        break;
    case WIFI_STATUS_ERROR:
        snprintf(buf, sizeof(buf), "WiFi: error");
        lv_obj_set_style_text_color(lbl_wifi_status, COL_RED, 0);
        break;
    default:
        snprintf(buf, sizeof(buf), "WiFi: disconnected");
        lv_obj_set_style_text_color(lbl_wifi_status, COL_DIM, 0);
        break;
    }
    lv_label_set_text(lbl_wifi_status, buf);
}

void ui_update_battery(int percent, bool charging) {
    int idx;
    if (charging)        idx = 4;
    else if (percent < 0) idx = 0;
    else if (percent <= 10) idx = 0;
    else if (percent <= 35) idx = 1;
    else if (percent <= 75) idx = 2;
    else                    idx = 3;
    lv_image_set_src(battery_img, &battery_dscs[idx]);
}

void ui_settings_get_values(char* ssid, int ssid_len,
                             char* pass, int pass_len,
                             char* api_key, int key_len) {
    strncpy(ssid,    lv_textarea_get_text(ta_ssid),    ssid_len    - 1);
    strncpy(pass,    lv_textarea_get_text(ta_pass),    pass_len    - 1);
    strncpy(api_key, lv_textarea_get_text(ta_api_key), key_len     - 1);
    ssid[ssid_len - 1] = pass[pass_len - 1] = api_key[key_len - 1] = '\0';
}

void ui_settings_set_values(const char* ssid, const char* pass, const char* api_key) {
    lv_textarea_set_text(ta_ssid,    ssid    ? ssid    : "");
    lv_textarea_set_text(ta_pass,    pass    ? pass    : "");
    lv_textarea_set_text(ta_api_key, api_key ? api_key : "");
}

void ui_settings_set_status(const char* msg) {
    lv_label_set_text(lbl_settings_status, msg ? msg : "");
}

bool ui_settings_save_requested(void) {
    if (!settings_save_requested) return false;
    settings_save_requested = false;
    return true;
}
