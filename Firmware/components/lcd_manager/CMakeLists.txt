idf_component_register(
    SRCS
        # Core runtime
        "src/lcd_manager.c"
        "src/ui_runtime.c"
        # UI structure
        "ui/views/dashboard_view.c"
        "ui/screens/splash_screen.c"
        # Generated LVGL glue
        "ui/generated/examples.c"
        "ui/generated/examples_gen.c"
        "ui/generated/screens/elements_gen.c"
        "ui/generated/components/basic/card/card_gen.c"
        "ui/generated/components/basic/div/div_gen.c"
        "ui/generated/components/basic/row/row_gen.c"
        "ui/generated/components/cards/thermostat/thermostat_gen.c"
        "ui/generated/hooks/ui_comp_hook.c"
        # Assets
        "assets/fonts/gist-14.c"
        "assets/fonts/gist-24.c"
        "assets/fonts/yekan_bakh_16.c"
        "assets/images/ai_logo.c"
        "assets/images/ui_img_device_thermostat_48dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
        "assets/images/ui_img_electric_meter_48dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
        "assets/images/ui_img_humidity_percentage_48dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
        "assets/images/ui_img_mode_fan_54dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
        "assets/animations/wave.c"
    INCLUDE_DIRS
        "include"
        "ui"
    PRIV_INCLUDE_DIRS
        "."
        "src"
        "ui/generated"
        "ui/views"
        "ui/screens"
        "assets/fonts"
    REQUIRES esp_lcd spiffs esp_psram lvgl esp_lvgl_port esp_timer)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE=1)
