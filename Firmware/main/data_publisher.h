#ifndef DATA_PUBLISHER_H
#define DATA_PUBLISHER_H

#include "sensor_manager.h"

/**
 * @brief Formats INA226 power data as JSON and publishes it via MQTT.
 *
 * @param data A pointer to the ina226_data_t struct containing the sensor reading.
 */
void publish_power_data(ina226_data_t *data);

/**
 * @brief Formats SHTC3 environment data as JSON and publishes it via MQTT.
 *
 * @param data A pointer to the shtc3_data_t struct containing the sensor reading.
 */
void publish_environment_data(shtc3_data_t *data);

/**
 * @brief Formats MPU6050 motion data as JSON and publishes it via MQTT.
 *
 * @param data A pointer to the mpu6050_data_t struct containing the sensor reading.
 */
void publish_motion_data(mpu6050_data_t *data);

void publish_slider_setpoint(uint8_t slider_percentage);

#endif // DATA_PUBLISHER_H