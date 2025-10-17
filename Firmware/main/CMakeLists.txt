idf_component_get_property(main_dir main "COMPONENT_DIR")

set(COMPONENT_SRCS "data_publisher.c" "main.c" "${main_dir}/main.c")
set(COMPONENT_ADD_INCLUDEDIRS "${main_dir}")

idf_component_register(SRCS ${COMPONENT_SRCS}

   INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
   REQUIRES network_manager nvs_flash sensor_manager motor_manager touch_slider lcd_manager business_logic led_manager json ui_controller)
