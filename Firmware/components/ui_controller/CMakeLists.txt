idf_component_register(SRCS "ui_controller.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos touch_slider led_manager esp_timer motor_manager business_logic lcd_manager)
