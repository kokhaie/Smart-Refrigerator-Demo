idf_component_register(SRCS "led_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES freertos esp_timer led_strip)
