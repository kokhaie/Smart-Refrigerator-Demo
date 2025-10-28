idf_component_get_property(main_dir main "COMPONENT_DIR")

set(COMPONENT_SRCS "main.c" "data_publisher.c")
set(COMPONENT_ADD_INCLUDEDIRS "${main_dir}")

idf_component_register(SRCS ${COMPONENT_SRCS}
   INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
   REQUIRES nvs_flash sensor_manager motor_manager anomaly_detector network_manager business_logic touch_slider led_manager ui_controller lcd_manager json)
