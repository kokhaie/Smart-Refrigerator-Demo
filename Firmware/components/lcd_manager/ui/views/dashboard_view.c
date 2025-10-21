#include <string.h>
#include "ui/ui.h"
#include "ui/generated/examples.h"
#include "ui/generated/screens/elements_gen.h"
#include "ui/generated/components/cards/thermostat/thermostat_gen.h"
#include "ui/views/dashboard_view.h"
#include "sdkconfig.h"

#ifndef CONFIG_LCD_UI_ASSET_BASE_PATH
#define CONFIG_LCD_UI_ASSET_BASE_PATH ""
#endif

#define UI_ARC_MIN_VALUE 0
#define UI_ARC_MAX_VALUE 8

static bool s_examples_initialized = false;
static bool s_dark_theme_enabled = true;
static int32_t s_room_temperature_c = 4;
static int32_t s_setpoint_temperature_c = 4;
static char s_mode_label[32] = "هوشمند • AI Auto";
static uint32_t s_mode_accent_rgb = 0x00C853;
static lcd_connectivity_state_t s_connectivity_state = LCD_CONNECTIVITY_STATE_OFFLINE;
static lv_obj_t *s_mode_badge = NULL;
static lv_obj_t *s_network_led = NULL;
static bool s_component_handles_cached = false;
static bool s_mode_style_override = false;

lv_obj_t *dashboard_view_root = NULL;

static void ensure_examples_initialized(void);
static int32_t clamp_ticks(int32_t value);
static void apply_subject_state(void);
static void cache_component_handles(void);
static void apply_mode_badge(void);
static void apply_connectivity_state(void);
static void network_led_anim_cb(void *obj, int32_t value);
static void start_network_led_animation(uint8_t from, uint8_t to, uint32_t duration_ms, bool playback);

static void ensure_examples_initialized(void)
{
    if (s_examples_initialized)
    {
        return;
    }

    examples_init(CONFIG_LCD_UI_ASSET_BASE_PATH);
    s_examples_initialized = true;
}

static int32_t clamp_ticks(int32_t value)
{
    if (value < UI_ARC_MIN_VALUE)
    {
        return UI_ARC_MIN_VALUE;
    }
    if (value > UI_ARC_MAX_VALUE)
    {
        return UI_ARC_MAX_VALUE;
    }
    return value;
}

static void apply_subject_state(void)
{
    lv_subject_set_int(&dark_theme, s_dark_theme_enabled ? 1 : 0);
    lv_subject_set_int(&room_temp, s_room_temperature_c);
    lv_subject_set_int(&setpoint_temp, s_setpoint_temperature_c);
    lv_subject_set_int(&thermostat_temp, clamp_ticks(s_setpoint_temperature_c));
}

static void cache_component_handles(void)
{
    if (!dashboard_view_root)
    {
        return;
    }

    lv_obj_t *card = lv_obj_get_child(dashboard_view_root, 0);
    if (!card)
    {
        return;
    }

    lv_obj_t *top_row = lv_obj_get_child(card, 0);
    if (!top_row)
    {
        return;
    }

    s_mode_badge = lv_obj_get_child(top_row, 0);
    s_network_led = lv_obj_get_child(top_row, 1);
    s_component_handles_cached = (s_mode_badge != NULL && s_network_led != NULL);
}

static void apply_mode_badge(void)
{
    if (!dashboard_view_root)
    {
        return;
    }

    if (!s_component_handles_cached || s_mode_badge == NULL)
    {
        cache_component_handles();
    }

    if (s_mode_badge == NULL)
    {
        return;
    }

    if (!s_mode_style_override)
    {
        return;
    }

    lv_label_set_text(s_mode_badge, s_mode_label);

    lv_color_t accent = lv_color_hex(s_mode_accent_rgb);
    lv_obj_set_style_bg_color(s_mode_badge, accent, 0);
    lv_obj_set_style_bg_opa(s_mode_badge, LV_OPA_40, 0);

    uint8_t r = (uint8_t)((s_mode_accent_rgb >> 16) & 0xFFU);
    uint8_t g = (uint8_t)((s_mode_accent_rgb >> 8) & 0xFFU);
    uint8_t b = (uint8_t)(s_mode_accent_rgb & 0xFFU);
    float luminance = 0.299f * (float)r + 0.587f * (float)g + 0.114f * (float)b;
    lv_color_t text_color = (luminance > 155.0f) ? lv_color_hex(0x0E0E0E) : lv_color_hex(0xFFFFFF);
    lv_obj_set_style_text_color(s_mode_badge, text_color, 0);
}

static void network_led_anim_cb(void *obj, int32_t value)
{
    lv_led_set_brightness((lv_obj_t *)obj, (uint8_t)value);
}

static void start_network_led_animation(uint8_t from, uint8_t to, uint32_t duration_ms, bool playback)
{
    if (!s_network_led)
    {
        return;
    }

    lv_anim_delete(s_network_led, network_led_anim_cb);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_network_led);
    lv_anim_set_values(&anim, from, to);
    lv_anim_set_time(&anim, duration_ms);
    if (playback)
    {
        lv_anim_set_playback_time(&anim, duration_ms);
    }
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, network_led_anim_cb);
    lv_anim_start(&anim);
}

static void apply_connectivity_state(void)
{
    if (!dashboard_view_root)
    {
        return;
    }

    if (!s_component_handles_cached || s_network_led == NULL)
    {
        cache_component_handles();
    }

    if (s_network_led == NULL)
    {
        return;
    }

    lv_anim_delete(s_network_led, network_led_anim_cb);

    switch (s_connectivity_state)
    {
    case LCD_CONNECTIVITY_STATE_OFFLINE:
        lv_led_set_color(s_network_led, lv_color_hex(0xFF4D4D));
        lv_led_set_brightness(s_network_led, 120);
        break;
    case LCD_CONNECTIVITY_STATE_CONNECTING:
        lv_led_set_color(s_network_led, lv_color_hex(0xFFB347));
        start_network_led_animation(30U, 200U, 480U, true);
        break;
    case LCD_CONNECTIVITY_STATE_ONLINE:
    default:
        lv_led_set_color(s_network_led, lv_color_hex(0x00E6A8));
        start_network_led_animation(60U, 200U, 1200U, true);
        break;
    }
}

void dashboard_view_init(void)
{
    ensure_examples_initialized();

    dashboard_view_root = elements_create();
    if (dashboard_view_root)
    {
        lv_obj_remove_flag(dashboard_view_root, LV_OBJ_FLAG_SCROLLABLE);
        apply_subject_state();
        cache_component_handles();
        apply_mode_badge();
        apply_connectivity_state();
    }
}

void dashboard_view_deinit(void)
{
    if (dashboard_view_root)
    {
        lv_obj_del(dashboard_view_root);
        dashboard_view_root = NULL;
    }
    s_mode_badge = NULL;
    s_network_led = NULL;
    s_component_handles_cached = false;
    s_mode_style_override = false;
}

void dashboard_view_set_dark_theme(bool enable)
{
    s_dark_theme_enabled = enable;
    if (dashboard_view_root)
    {
        apply_subject_state();
    }
}

void dashboard_view_set_room_temperature(int32_t temperature_c)
{
    s_room_temperature_c = temperature_c;
    if (dashboard_view_root)
    {
        apply_subject_state();
    }
}

void dashboard_view_set_target_temperature(int32_t temperature_c)
{
    s_setpoint_temperature_c = temperature_c;
    if (dashboard_view_root)
    {
        apply_subject_state();
    }
}

void dashboard_view_set_mode_display(const char *label, uint32_t accent_rgb24)
{
    if (label != NULL)
    {
        strncpy(s_mode_label, label, sizeof(s_mode_label) - 1U);
        s_mode_label[sizeof(s_mode_label) - 1U] = '\0';
    }

    s_mode_accent_rgb = accent_rgb24;
    s_mode_style_override = true;

    if (dashboard_view_root)
    {
        apply_mode_badge();
    }
}

void dashboard_view_set_connectivity_state(lcd_connectivity_state_t state)
{
    s_connectivity_state = state;

    if (dashboard_view_root)
    {
        apply_connectivity_state();
    }
}
