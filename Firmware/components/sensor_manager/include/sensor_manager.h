#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

// ------------------------- Config -------------------------
#define BATCH_SIZE 1000

// ------------------------- Data Structures -------------------------

// INA226 (Power)
typedef struct
{
    float bus_voltage_v;
    float current_a;
} ina226_data_t;

// SHTC3 (Temp/Humidity)
typedef struct
{
    float temperature_c;
    float humidity_rh;
} shtc3_data_t;

// MPU6050 (Motion)
typedef struct
{
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

// Unified synchronized sample
typedef struct
{
    uint64_t timestamp_us;
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float latest_current_a;
    float latest_temperature_c;
    float magnitude;
} synchronized_sample_t;

// ------------------------- API -------------------------

/**
 * @brief Initialize the sensor manager.
 * Creates I2C bus, initializes sensors, and starts tasks.
 */
esp_err_t sensor_manager_init(void);

/**
 * @brief Get the next decimated sample (for real-time stream/MQTT/graphs).
 * @param out Pointer to store the sample
 * @param timeout Timeout ticks to wait
 * @return true if a sample was retrieved, false otherwise
 */
bool sensor_manager_get_next_sample(synchronized_sample_t *out, TickType_t timeout);

/**
 * @brief Get the next raw 1 kHz sample.
 * @param out Pointer to store the sample
 * @param timeout Timeout ticks to wait
 * @return true if a sample was retrieved, false otherwise
 */
bool sensor_manager_get_raw_sample(synchronized_sample_t *out, TickType_t timeout);

/**
 * @brief Get the latest full batch of samples (for AI training).
 * @param out_batch Pointer to buffer to hold the batch (must be size BATCH_SIZE)
 * @param out_count Returns the number of samples (should always be BATCH_SIZE)
 * @param timeout Timeout ticks to wait
 * @return true if a batch was retrieved, false otherwise
 */
bool sensor_manager_get_batch(synchronized_sample_t *out_batch, int *out_count, TickType_t timeout);

/**
 * @brief Get the latest environment data (temperature/humidity).
 * @param out Pointer to shtc3_data_t struct
 * @return true if valid data was retrieved, false otherwise
 */
bool sensor_manager_get_latest_environment(shtc3_data_t *out);

#endif // SENSOR_MANAGER_H
