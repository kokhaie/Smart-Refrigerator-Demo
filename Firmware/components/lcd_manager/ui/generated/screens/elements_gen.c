/**
 * @file elements_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "elements_gen.h"
#include "ui/generated/examples.h"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * elements_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_dark;
    static lv_style_t style_light;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_dark);
        lv_style_set_bg_color(&style_dark, lv_color_hex(0x000000));
        lv_style_set_bg_opa(&style_dark, LV_OPA_COVER);

        lv_style_init(&style_light);
        lv_style_set_bg_color(&style_light, ACCENT2_50_LIGHT);
        lv_style_set_bg_opa(&style_light, LV_OPA_COVER);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_remove_style_all(lv_obj_0);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_width(lv_obj_0, lv_pct(100));
    lv_obj_set_height(lv_obj_0, lv_pct(100));

    lv_obj_bind_style(lv_obj_0, &style_light, 0, &dark_theme, 0);
    lv_obj_bind_style(lv_obj_0, &style_dark, 0, &dark_theme, 1);
    lv_obj_t * thermostat_0 = thermostat_create(lv_obj_0);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "elements");

    return lv_obj_0;
}
