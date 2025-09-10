idf_component_register(SRCS "network_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt esp_wifi esp_event nvs_flash log
)
