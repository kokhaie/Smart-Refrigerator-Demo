idf_component_register(SRCS
                    # --- Driver and Porting Layer ---
                    "ui_manager.c"
                    # --- SquareLine Studio Generated UI ---
                    "ui.c"
                    "ui_helpers.c"
                    "screens/ui_Screen1.c"
                    "screens/splash_screen.c"
                    "components/ui_comp_hook.c"
                    "anim/ailogo.c"
                    "anim/wave.c"
                    "images/ui_img_device_thermostat_48dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
                    "images/ui_img_electric_meter_48dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
                    "images/ui_img_humidity_percentage_48dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
                    "images/ui_img_mode_fan_54dp_ffffff_fill1_wght400_grad0_opsz48_png.c"
                    "fonts/ui_font_yekanbold24.c"
                    "fonts/ui_font_yekanreg22.c"
                    # --- Public and Private Include Directories ---
                    INCLUDE_DIRS
                    "include"  # For public headers like ui_manager.h
                    "."        # For top-level headers like ui.h
                    "screens"
                    "components"
                    "anim"
                    "images"
                    "fonts"
                    # --- Component Dependencies ---
                    REQUIRES esp_lcd spiffs esp_psram lvgl esp_timer)
