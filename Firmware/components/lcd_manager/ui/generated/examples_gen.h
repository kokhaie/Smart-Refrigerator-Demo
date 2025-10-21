/**
 * @file examples_gen.h
 */

#ifndef EXAMPLES_GEN_H
#define EXAMPLES_GEN_H

#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

#define UNIT_SM 6
#define UNIT_MD 12
#define UNIT_LG 18
#define UNIT_XL 24
#define OPA_MUTED lv_pct(20)

#define LIGHT lv_color_hex(0xffffff)
#define DARK lv_color_hex(0x0e0e0e)
#define SURFACE_PRIMARY_LIGHT lv_color_hex(0x0e0e0e)
#define TEXT_ON_SURFACE_PRIMARY_LIGHT lv_color_hex(0xffffff)
#define SURFACE_PRIMARY_DARK lv_color_hex(0xffffff)
#define TEXT_ON_SURFACE_PRIMARY_DARK lv_color_hex(0x0e0e0e)
#define BG_PRIMARY_LIGHT lv_color_hex(0xffffff)
#define BG_PRIMARY_DARK lv_color_hex(0x0e0e0e)
#define BG_SECONDARY_LIGHT lv_color_hex(0xf0f0f0)
#define BG_SECONDARY_DARK lv_color_hex(0x000000)
#define BG_TERTIARY_LIGHT lv_color_hex(0xf0f0f0)
#define BG_TERTIARY_DARK lv_color_hex(0x373130)
#define ACCENT1_LIGHT lv_color_hex(0xAF4ADE)
#define ACCENT1_DARK lv_color_hex(0xAF4ADE)
#define ACCENT1_50_LIGHT lv_color_hex(0xD2B1F6)
#define ACCENT1_50_DARK lv_color_hex(0x7E4CB7)
#define ACCENT2_LIGHT lv_color_hex(0xe9deaf)
#define ACCENT2_DARK lv_color_hex(0x887A3D)
#define ACCENT2_50_LIGHT lv_color_hex(0xf3f0e7)
#define ACCENT2_50_DARK lv_color_hex(0x4A473E)

/**********************
 * GLOBAL VARIABLES
 **********************/

extern lv_style_t style_disabled;
extern lv_style_t style_reset;
extern lv_style_t figma_import_test;

extern const lv_font_t *geist_semibold_12;
extern const lv_font_t *geist_semibold_14;
extern const lv_font_t *geist_bold_16;
extern const lv_font_t *geist_semibold_20;
extern const lv_font_t *geist_semibold_28;
extern const lv_font_t *geist_regular_12;
extern const lv_font_t *geist_regular_14;
extern const lv_font_t *geist_light_60;
extern const lv_font_t *literata_80;
extern const lv_font_t *abril_fatface_80;
extern const lv_font_t *big_shoulders_80;

extern const void * icon_plus;
extern const void * icon_minus;
extern const void * light_temp_arc_bg;
extern const void * icon_heart;
extern const void * icon_play;
extern const void * icon_pause;
extern const void * icon_skip_back;
extern const void * icon_skip_forward;
extern const void * icon_volume_max;
extern const void * icon_volume_min;
extern const void * icon_volume_none;
extern const void * song_cover_1;
extern const void * weather_location_1_bg;
extern const void * weather_location_2_bg;
extern const void * icon_cloudy;
extern const void * icon_sunny;
extern const void * icon_pin;
extern const void * icon_theme;

extern lv_subject_t dark_theme;
extern lv_subject_t move_goal_target;
extern lv_subject_t location1_temp;
extern lv_subject_t location2_temp;
extern lv_subject_t thermostat_on;
extern lv_subject_t thermostat_temp;
extern lv_subject_t room_temp;
extern lv_subject_t setpoint_temp;
extern lv_subject_t alarm_on;
extern lv_subject_t alarm_hour;
extern lv_subject_t alarm_min;
extern lv_subject_t speaker;
extern lv_subject_t speaker_vol;
extern lv_subject_t light_temperature;
extern lv_subject_t light_temperature_temp;
extern lv_subject_t song_played;
extern lv_subject_t song_liked;
extern lv_subject_t song_playing;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void examples_init_gen(const char * asset_path);

/**********************
 *   POST INCLUDES
 **********************/

#include "components/basic/card/card_gen.h"
#include "components/basic/row/row_gen.h"
#include "components/basic/div/div_gen.h"
#include "components/cards/thermostat/thermostat_gen.h"
#include "screens/elements_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*EXAMPLES_GEN_H*/
