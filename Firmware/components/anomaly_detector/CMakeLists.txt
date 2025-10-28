idf_component_get_property(main_dir main "COMPONENT_DIR")

idf_component_register(SRCS "fault_simulator.c" "data_collector.c" "anomaly_detector.c"
                    INCLUDE_DIRS "include" "${main_dir}"
                    REQUIRES sensor_manager nvs_flash motor_manager)
