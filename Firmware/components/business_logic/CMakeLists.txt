idf_component_register(SRCS "business_logic.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_timer motor_manager sensor_manager)
