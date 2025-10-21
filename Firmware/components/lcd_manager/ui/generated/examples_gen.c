/**
 * @file examples_gen.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "examples_gen.h"

/* Use project fonts provided by the lcd_manager component */
LV_FONT_DECLARE(font_geist_14);
LV_FONT_DECLARE(font_geist_24);
LV_FONT_DECLARE(font_yekan_bakh_16);

/**********************
 *  GLOBAL VARIABLES
 **********************/

lv_style_t style_disabled;
lv_style_t style_reset;
lv_style_t figma_import_test;

const lv_font_t *geist_semibold_12;
const lv_font_t *geist_semibold_14;
const lv_font_t *geist_bold_16;
const lv_font_t *geist_semibold_20;
const lv_font_t *geist_semibold_28;
const lv_font_t *geist_regular_12;
const lv_font_t *geist_regular_14;
const lv_font_t *geist_light_60;
const lv_font_t *literata_80;
const lv_font_t *abril_fatface_80;
const lv_font_t *big_shoulders_80;

const void * icon_plus;
const void * icon_minus;
const void * light_temp_arc_bg;
const void * icon_heart;
const void * icon_play;
const void * icon_pause;
const void * icon_skip_back;
const void * icon_skip_forward;
const void * icon_volume_max;
const void * icon_volume_min;
const void * icon_volume_none;
const void * song_cover_1;
const void * weather_location_1_bg;
const void * weather_location_2_bg;
const void * icon_cloudy;
const void * icon_sunny;
const void * icon_pin;
const void * icon_theme;

lv_subject_t dark_theme;
lv_subject_t move_goal_target;
lv_subject_t location1_temp;
lv_subject_t location2_temp;
lv_subject_t thermostat_on;
lv_subject_t thermostat_temp;
lv_subject_t room_temp;
lv_subject_t setpoint_temp;
lv_subject_t alarm_on;
lv_subject_t alarm_hour;
lv_subject_t alarm_min;
lv_subject_t speaker;
lv_subject_t speaker_vol;
lv_subject_t light_temperature;
lv_subject_t light_temperature_temp;
lv_subject_t song_played;
lv_subject_t song_liked;
lv_subject_t song_playing;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void examples_init_gen(const char * asset_path)
{
    char buf[256];

    /*----------------
     * Global styles
     *----------------*/

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_disabled);
        lv_style_set_opa_layered(&style_disabled, (255 * 60 / 100));

        lv_style_init(&style_reset);
        lv_style_set_width(&style_reset, LV_SIZE_CONTENT);
        lv_style_set_height(&style_reset, LV_SIZE_CONTENT);
        lv_style_set_bg_opa(&style_reset, 0);
        lv_style_set_border_width(&style_reset, 0);
        lv_style_set_radius(&style_reset, 0);
        lv_style_set_pad_all(&style_reset, 0);

        lv_style_init(&figma_import_test);
        lv_style_set_width(&figma_import_test, 24);
        lv_style_set_height(&figma_import_test, 24);

        style_inited = true;
    }

    /*----------------
     * Fonts
     *----------------*/

    LV_UNUSED(asset_path);

    /* Map all generated font slots to the project's bundled fonts */
    geist_semibold_12 = &font_geist_14;
    geist_semibold_14 = &font_geist_14;
    geist_bold_16 = &font_geist_14;
    geist_semibold_20 = &font_geist_14;
    geist_semibold_28 = &font_geist_24;
    geist_regular_12 = &font_geist_14;
    geist_regular_14 = &font_geist_14;
    geist_light_60 = &font_geist_24;
    literata_80 = &font_geist_24;
    abril_fatface_80 = &font_geist_24;
    big_shoulders_80 = &font_geist_24;

    /*----------------
     * Images
     *----------------*/
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_plus.png");
    icon_plus = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_minus.png");
    icon_minus = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/light_temp_arc_bg.png");
    light_temp_arc_bg = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_heart.png");
    icon_heart = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_play.png");
    icon_play = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_pause.png");
    icon_pause = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_skip_back.png");
    icon_skip_back = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_skip_forward.png");
    icon_skip_forward = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_volume_max.png");
    icon_volume_max = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_volume_min.png");
    icon_volume_min = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_volume_none.png");
    icon_volume_none = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/song_cover_1.png");
    song_cover_1 = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/weather_location_1_bg.png");
    weather_location_1_bg = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/weather_location_2_bg.png");
    weather_location_2_bg = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_cloudy.png");
    icon_cloudy = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_sunny.png");
    icon_sunny = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_pin.png");
    icon_pin = lv_strdup(buf);
    lv_snprintf(buf, sizeof(buf), "%s%s", asset_path, "assets/images/icon_theme.png");
    icon_theme = lv_strdup(buf);

    /*----------------
     * Subjects
     *----------------*/
    lv_subject_init_int(&dark_theme, 1);
    lv_subject_init_int(&move_goal_target, 800);
    lv_subject_set_min_value_int(&move_goal_target, 0);
    lv_subject_set_max_value_int(&move_goal_target, 2000);
    lv_subject_init_int(&location1_temp, 25);
    lv_subject_init_int(&location2_temp, 34);
    lv_subject_init_int(&thermostat_on, 1);
    lv_subject_init_int(&thermostat_temp, 4);
    lv_subject_init_int(&room_temp, 4);
    lv_subject_init_int(&setpoint_temp, 4);
    lv_subject_init_int(&alarm_on, 1);
    lv_subject_init_int(&alarm_hour, 6);
    lv_subject_init_int(&alarm_min, 36);
    lv_subject_init_int(&speaker, 1);
    lv_subject_init_int(&speaker_vol, 40);
    lv_subject_init_int(&light_temperature, 1);
    lv_subject_init_int(&light_temperature_temp, 3000);
    lv_subject_init_int(&song_played, 130);
    lv_subject_init_int(&song_liked, 0);
    lv_subject_init_int(&song_playing, 0);

#if LV_USE_XML
    /* Register fonts */
    lv_xml_register_font(NULL, "geist_semibold_12", geist_semibold_12);
    lv_xml_register_font(NULL, "geist_semibold_14", geist_semibold_14);
    lv_xml_register_font(NULL, "geist_bold_16", geist_bold_16);
    lv_xml_register_font(NULL, "geist_semibold_20", geist_semibold_20);
    lv_xml_register_font(NULL, "geist_semibold_28", geist_semibold_28);
    lv_xml_register_font(NULL, "geist_regular_12", geist_regular_12);
    lv_xml_register_font(NULL, "geist_regular_14", geist_regular_14);
    lv_xml_register_font(NULL, "geist_light_60", geist_light_60);
    lv_xml_register_font(NULL, "literata_80", literata_80);
    lv_xml_register_font(NULL, "abril_fatface_80", abril_fatface_80);
    lv_xml_register_font(NULL, "big_shoulders_80", big_shoulders_80);

    /* Register subjects */
    lv_xml_register_subject(NULL, "dark_theme", &dark_theme);
    lv_xml_register_subject(NULL, "move_goal_target", &move_goal_target);
    lv_xml_register_subject(NULL, "location1_temp", &location1_temp);
    lv_xml_register_subject(NULL, "location2_temp", &location2_temp);
    lv_xml_register_subject(NULL, "thermostat_on", &thermostat_on);
    lv_xml_register_subject(NULL, "thermostat_temp", &thermostat_temp);
    lv_xml_register_subject(NULL, "room_temp", &room_temp);
    lv_xml_register_subject(NULL, "setpoint_temp", &setpoint_temp);
    lv_xml_register_subject(NULL, "alarm_on", &alarm_on);
    lv_xml_register_subject(NULL, "alarm_hour", &alarm_hour);
    lv_xml_register_subject(NULL, "alarm_min", &alarm_min);
    lv_xml_register_subject(NULL, "speaker", &speaker);
    lv_xml_register_subject(NULL, "speaker_vol", &speaker_vol);
    lv_xml_register_subject(NULL, "light_temperature", &light_temperature);
    lv_xml_register_subject(NULL, "light_temperature_temp", &light_temperature_temp);
    lv_xml_register_subject(NULL, "song_played", &song_played);
    lv_xml_register_subject(NULL, "song_liked", &song_liked);
    lv_xml_register_subject(NULL, "song_playing", &song_playing);
#endif
}
