/**
 * @file thermostat_gen.c
 * @brief Modern thermostat UI with improved UX
 */

/*********************
 *      INCLUDES
 *********************/

#include "thermostat_gen.h"
#include "ui/generated/examples.h"

static void led_brightness_anim_cb(void *obj, int32_t value)
{
    lv_led_set_brightness((lv_obj_t *)obj, (uint8_t)value);
}

static void start_led_breathing(lv_obj_t *led)
{
    lv_led_set_brightness(led, 40);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, led);
    lv_anim_set_values(&anim, 40, 200);
    lv_anim_set_time(&anim, 600);
    lv_anim_set_playback_time(&anim, 600);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&anim, led_brightness_anim_cb);
    lv_anim_start(&anim);
}

LV_FONT_DECLARE(font_geist_14);
LV_FONT_DECLARE(font_geist_24);
LV_FONT_DECLARE(font_yekan_bakh_16);
LV_IMG_DECLARE(ai_logo_data);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *thermostat_create(lv_obj_t *parent)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_center;
    static lv_style_t style_center_dark;
    static lv_style_t style_arc_no_line;
    static lv_style_t style_arc_knob;
    static lv_style_t style_scale_main;
    static lv_style_t style_scale_main_dark;
    static lv_style_t style_scale_ticks;
    static lv_style_t style_scale_ticks_dark;
    static lv_style_t style_scale_section_ticks;
    static lv_style_t style_badge;
    static lv_style_t style_badge_dark;

    static bool style_inited = false;

    if (!style_inited)
    {
        // Center circle with modern glassmorphism effect
        lv_style_init(&style_center);
        lv_style_set_width(&style_center, 140);
        lv_style_set_height(&style_center, 140);
        lv_style_set_radius(&style_center, 100);
        lv_style_set_bg_color(&style_center, BG_PRIMARY_LIGHT);
        lv_style_set_border_width(&style_center, 0);
        lv_style_set_shadow_color(&style_center, DARK);
        lv_style_set_shadow_offset_y(&style_center, 8);
        lv_style_set_shadow_opa(&style_center, 40);
        lv_style_set_shadow_spread(&style_center, -4);
        lv_style_set_shadow_width(&style_center, 24);

        lv_style_init(&style_center_dark);
        lv_style_set_bg_color(&style_center_dark, BG_TERTIARY_DARK);
        lv_style_set_text_color(&style_center_dark, SURFACE_PRIMARY_DARK);

        lv_style_init(&style_arc_no_line);
        lv_style_set_arc_width(&style_arc_no_line, 0);

        // Enhanced knob with more visible design
        lv_style_init(&style_arc_knob);
        lv_style_set_pad_all(&style_arc_knob, 6);
        lv_style_set_bg_color(&style_arc_knob, ACCENT1_LIGHT);
        lv_style_set_shadow_color(&style_arc_knob, ACCENT1_LIGHT);
        lv_style_set_shadow_width(&style_arc_knob, 8);
        lv_style_set_shadow_opa(&style_arc_knob, 100);
        lv_style_set_shadow_spread(&style_arc_knob, 0);

        lv_style_init(&style_scale_main);
        lv_style_set_width(&style_scale_main, 160);
        lv_style_set_height(&style_scale_main, 160);
        lv_style_set_arc_width(&style_scale_main, 0);
        lv_style_set_text_font(&style_scale_main, geist_semibold_12);
        lv_style_set_text_color(&style_scale_main, SURFACE_PRIMARY_LIGHT);

        lv_style_init(&style_scale_main_dark);
        lv_style_set_text_color(&style_scale_main_dark, SURFACE_PRIMARY_DARK);

        lv_style_init(&style_scale_ticks);
        lv_style_set_line_color(&style_scale_ticks, SURFACE_PRIMARY_LIGHT);
        lv_style_set_line_width(&style_scale_ticks, 3);
        lv_style_set_line_opa(&style_scale_ticks, (255 * 50 / 100));
        lv_style_set_length(&style_scale_ticks, 8);

        lv_style_init(&style_scale_ticks_dark);
        lv_style_set_line_color(&style_scale_ticks_dark, SURFACE_PRIMARY_DARK);

        // Enhanced active section with gradient effect
        lv_style_init(&style_scale_section_ticks);
        lv_style_set_line_color(&style_scale_section_ticks, ACCENT1_DARK);
        lv_style_set_line_width(&style_scale_section_ticks, 14);
        lv_style_set_line_opa(&style_scale_section_ticks, 255);

        // Modern badge style with subtle backdrop
        lv_style_init(&style_badge);
        lv_style_set_border_width(&style_badge, 1);
        lv_style_set_border_opa(&style_badge, 40);
        lv_style_set_radius(&style_badge, 12);
        lv_style_set_bg_opa(&style_badge, 30);
        lv_style_set_pad_left(&style_badge, 10);
        lv_style_set_pad_right(&style_badge, 10);
        lv_style_set_pad_top(&style_badge, 4);
        lv_style_set_pad_bottom(&style_badge, 4);

        lv_style_init(&style_badge_dark);
        lv_style_set_bg_opa(&style_badge_dark, 50);

        style_inited = true;
    }

    lv_obj_t *card_0 = card_create(parent);
    lv_obj_set_width(card_0, lv_pct(100));
    lv_obj_set_height(card_0, lv_pct(100));

    // Top row with status
    lv_obj_t *row_0 = row_create(card_0);
    lv_obj_set_style_flex_main_place(row_0, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_width(row_0, lv_pct(100));
    lv_obj_set_style_margin_top(row_0, -15, 0);
    lv_obj_set_style_pad_left(row_0, 5, 0);
    lv_obj_set_style_pad_right(row_0, 5, 0);
    lv_obj_set_style_pad_top(row_0, 5, 0);

    lv_obj_t *lv_label_0 = lv_label_create(row_0);
    lv_label_set_text(lv_label_0, "هوشمند • AI Auto");
    lv_obj_add_style(lv_label_0, &style_badge, 0);
    lv_obj_bind_style(lv_label_0, &style_badge_dark, 0, &dark_theme, 1);
    lv_obj_set_style_text_font(lv_label_0, &font_yekan_bakh_16, 0);

    lv_obj_t *status_dot_0 = lv_led_create(row_0);
    lv_obj_set_size(status_dot_0, 12, 12);
    lv_obj_set_style_pad_top(status_dot_0, 4, 0);
    lv_led_set_color(status_dot_0, lv_color_hex(0x00C853));
    start_led_breathing(status_dot_0);

    // Center thermostat control
    lv_obj_t *div_0 = div_create(card_0);
    lv_obj_set_style_layout(div_0, LV_LAYOUT_NONE, 0);
    lv_obj_set_style_pad_top(div_0, 3, 0);
    lv_obj_set_style_margin_top(div_0, -20, 0);

    lv_obj_t *center = lv_obj_create(div_0);
    lv_obj_set_name(center, "center");
    lv_obj_set_align(center, LV_ALIGN_CENTER);
    lv_obj_add_style(center, &style_center, 0);
    lv_obj_bind_style(center, &style_center_dark, 0, &dark_theme, 1);

    // Large temperature display
    lv_obj_t *lv_label_1 = lv_label_create(center);
    lv_label_bind_text(lv_label_1, &setpoint_temp, "%d°");
    lv_obj_set_style_text_font(lv_label_1, &font_geist_24, 0);
    lv_obj_set_align(lv_label_1, LV_ALIGN_CENTER);
    lv_obj_set_style_margin_top(lv_label_1, -8, 0);

    // Room temperature with cleaner format
    lv_obj_t *lv_label_2 = lv_label_create(center);
    lv_label_bind_text(lv_label_2, &room_temp, "Room: %d°C");
    lv_obj_set_style_text_font(lv_label_2, &font_geist_14, 0);
    lv_obj_set_align(lv_label_2, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_pad_bottom(lv_label_2, 10, 0);
    lv_obj_set_style_text_opa(lv_label_2, 180, 0);

    // Scale with enhanced visibility
    lv_obj_t *thermostat_scale_light = lv_scale_create(div_0);
    lv_obj_set_name(thermostat_scale_light, "thermostat_scale_light");
    lv_obj_set_align(thermostat_scale_light, LV_ALIGN_CENTER);
    lv_scale_set_major_tick_every(thermostat_scale_light, 8);
    lv_scale_set_total_tick_count(thermostat_scale_light, 9);
    lv_scale_set_mode(thermostat_scale_light, LV_SCALE_MODE_ROUND_OUTER);
    lv_scale_set_angle_range(thermostat_scale_light, 180);
    lv_scale_set_min_value(thermostat_scale_light, 0);
    lv_scale_set_max_value(thermostat_scale_light, 8);
    lv_scale_set_rotation(thermostat_scale_light, 180);
    lv_obj_add_style(thermostat_scale_light, &style_scale_main, 0);
    lv_obj_bind_style(thermostat_scale_light, &style_scale_main_dark, 0, &dark_theme, 1);
    lv_obj_add_style(thermostat_scale_light, &style_scale_ticks, LV_PART_ITEMS);
    lv_obj_add_style(thermostat_scale_light, &style_scale_ticks, LV_PART_INDICATOR);
    lv_obj_bind_style(thermostat_scale_light, &style_scale_ticks_dark, LV_PART_ITEMS, &dark_theme, 1);
    lv_obj_bind_style(thermostat_scale_light, &style_scale_ticks_dark, LV_PART_INDICATOR, &dark_theme, 1);

    lv_scale_section_t *lv_scale_section_0 = lv_scale_add_section(thermostat_scale_light);
    lv_scale_set_section_min_value(thermostat_scale_light, lv_scale_section_0, 0);
    lv_scale_bind_section_max_value(thermostat_scale_light, lv_scale_section_0, &thermostat_temp);
    lv_scale_set_section_style_items(thermostat_scale_light, lv_scale_section_0, &style_scale_section_ticks);
    lv_scale_set_section_style_indicator(thermostat_scale_light, lv_scale_section_0, &style_scale_section_ticks);

    lv_obj_t *lv_arc_0 = lv_arc_create(div_0);
    lv_obj_set_width(lv_arc_0, 120);
    lv_obj_set_height(lv_arc_0, 120);
    lv_obj_set_ext_click_area(lv_arc_0, 20);
    lv_obj_set_align(lv_arc_0, LV_ALIGN_CENTER);
    lv_arc_bind_value(lv_arc_0, &thermostat_temp);
    lv_arc_set_min_value(lv_arc_0, 0);
    lv_arc_set_max_value(lv_arc_0, 8);
    lv_arc_set_bg_start_angle(lv_arc_0, 180);
    lv_arc_set_bg_end_angle(lv_arc_0, 360);
    lv_arc_set_start_angle(lv_arc_0, 180);
    lv_obj_add_style(lv_arc_0, &style_arc_no_line, LV_PART_MAIN);
    lv_obj_add_style(lv_arc_0, &style_arc_no_line, LV_PART_INDICATOR);
    lv_obj_add_style(lv_arc_0, &style_arc_knob, LV_PART_KNOB);

    // Bottom info row with better spacing
    lv_obj_t *row_1 = row_create(card_0);
    lv_obj_set_style_flex_main_place(row_1, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_width(row_1, lv_pct(100));
    lv_obj_set_style_margin_top(row_1, -15, 0);
    lv_obj_set_style_pad_left(row_1, 5, 0);
    lv_obj_set_style_pad_right(row_1, 5, 0);
    lv_obj_set_style_pad_top(row_1, 0, 0);
    lv_obj_set_style_margin_top(row_1,-25,0);

    // Badge 1: Health Score (keep as is)
    lv_obj_t *lv_label_3 = lv_label_create(row_1);
    lv_label_set_text(lv_label_3, "98%");
    lv_obj_add_style(lv_label_3, &style_badge, 0);
    lv_obj_bind_style(lv_label_3, &style_badge_dark, 0, &dark_theme, 1);
    lv_obj_set_style_text_font(lv_label_3, &font_geist_14, 0);
    lv_obj_set_style_pad_left(lv_label_3, 14, 0);
    lv_obj_set_style_pad_right(lv_label_3, 14, 0);

    // Badge 3: AI Logo (keep as is)
    lv_obj_t *ai_logo = lv_image_create(row_1);
    lv_image_set_src(ai_logo, &ai_logo_data);
    lv_image_set_scale(ai_logo, 384);
    lv_obj_clear_flag(ai_logo, LV_OBJ_FLAG_SCROLLABLE);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(card_0, "thermostat_#");

    return card_0;
}
