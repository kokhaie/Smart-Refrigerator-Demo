#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "esp_err.h"

// --- Data Structures for Sensor Readings ---

// Holds data from the INA226 Power Monitor
typedef struct {
    float bus_voltage_v;
    float current_a;
} ina226_data_t;

// Holds data from the SHTC3 Temp/Humidity Sensor
typedef struct {
    float temperature_c;
    float humidity_rh;
} shtc3_data_t;

// Holds data from the MPU6050 IMU
typedef struct {
    float accel_x_g, accel_y_g, accel_z_g;
    float gyro_x_dps, gyro_y_dps, gyro_z_dps;
    float pitch, roll;
} mpu6050_data_t;


// --- Public Functions ---

/**
 * @brief Initializes the I2C bus and all connected sensors.
 * This should be called once at application startup.
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Reads the latest data from the INA226 Power Monitor.
 *
 * @param data Pointer to a struct to store the reading.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sensor_manager_read_ina226(ina226_data_t *data);

/**
 * @brief Reads the latest data from the SHTC3 Temp/Humidity Sensor.
 *
 * @param data Pointer to a struct to store the reading.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sensor_manager_read_shtc3(shtc3_data_t *data);

/**
 * @brief Reads the latest data from the MPU6050 IMU.
 *
 * @param data Pointer to a struct to store the reading.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t sensor_manager_read_mpu6050(mpu6050_data_t *data);

#endif