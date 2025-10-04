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

typedef struct
{
    int32_t accel_x;
    int32_t accel_y;
    int32_t accel_z;
    int32_t gyro_x;
    int32_t gyro_y;
    int32_t gyro_z;
} mpu6050_offsets_t;

/**
 * @brief Initializes the I2C bus and all sensors, and starts the background reading tasks.
 * * @return esp_err_t ESP_OK on success, otherwise an error code.
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Thread-safely gets the latest data from the MPU6050 sensor.
 * * @param data Pointer to a struct where the data will be copied.
 * @return esp_err_t ESP_OK on success, ESP_FAIL if the data could not be retrieved.
 */
esp_err_t sensor_manager_get_latest_mpu6050_data(mpu6050_data_t *data);

/**
 * @brief Thread-safely gets the latest data from the INA226 sensor.
 * * @param data Pointer to a struct where the data will be copied.
 * @return esp_err_t ESP_OK on success, ESP_FAIL if the data could not be retrieved.
 */
esp_err_t sensor_manager_get_latest_ina226_data(ina226_data_t *data);

/**
 * @brief Thread-safely gets the latest data from the SHTC3 sensor.
 * * @param data Pointer to a struct where the data will be copied.
 * @return esp_err_t ESP_OK on success, ESP_FAIL if the data could not be retrieved.
 */
esp_err_t sensor_manager_get_latest_shtc3_data(shtc3_data_t *data);


#endif // SENSOR_MANAGER_H