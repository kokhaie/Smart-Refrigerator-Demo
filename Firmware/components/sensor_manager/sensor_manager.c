#include "sensor_manager.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "SENSOR_MANAGER";

// --- Private Handles and State Variables ---
static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t mpu6050_handle;
static i2c_master_dev_handle_t ina226_handle;
static i2c_master_dev_handle_t shtc3_handle;
static float ina226_current_lsb; // Stores the calculated LSB for INA226

// --- Hardware and Protocol Defines ---
#define I2C_MASTER_SCL_IO 16
#define I2C_MASTER_SDA_IO 15
#define I2C_MASTER_FREQ_HZ 100000

// MPU6050 Defines
#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B
#define ACCEL_SENSITIVITY 16384.0
#define GYRO_SENSITIVITY 131.0
#define RAD_TO_DEG (180.0 / M_PI)

// INA226 Defines
#define INA226_DEVICE_ADDRESS 0x40
#define INA226_REG_CONFIG 0x00
#define INA226_REG_BUSVOLTAGE 0x02
#define INA226_REG_CURRENT 0x04
#define INA226_REG_CALIBRATION 0x05
#define INA226_DEFAULT_CONFIG 0x4127
#define SHUNT_RESISTANCE_OHMS 0.1
#define MAX_EXPECTED_CURRENT_AMPS 3.2

// SHTC3 Defines
#define SHTC3_SENSOR_ADDR 0x70
#define SHTC3_CMD_WAKEUP 0x3517
#define SHTC3_CMD_SLEEP 0xB098
#define SHTC3_CMD_MEASURE_T_FIRST 0x7CA2
#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF

typedef struct
{
    int32_t accel_x;
    int32_t accel_y;
    int32_t accel_z;
    int32_t gyro_x;
    int32_t gyro_y;
    int32_t gyro_z;
} mpu6050_offsets_t;

static mpu6050_offsets_t mpu_offsets = {0};
static void sensor_manager_calibrate_mpu6050()
{
    ESP_LOGI(TAG, "Calibrating MPU6050... Keep the sensor flat and still.");
    uint8_t reg = MPU6050_ACCEL_XOUT_H;
    uint8_t buffer[14];
    const int num_readings = 500;

    for (int i = 0; i < num_readings; i++)
    {
        if (i2c_master_transmit_receive(mpu6050_handle, &reg, 1, buffer, 14, -1) == ESP_OK)
        {
            mpu_offsets.accel_x += (int16_t)((buffer[0] << 8) | buffer[1]);
            mpu_offsets.accel_y += (int16_t)((buffer[2] << 8) | buffer[3]);
            mpu_offsets.accel_z += (int16_t)((buffer[4] << 8) | buffer[5]);
            mpu_offsets.gyro_x += (int16_t)((buffer[8] << 8) | buffer[9]);
            mpu_offsets.gyro_y += (int16_t)((buffer[10] << 8) | buffer[11]);
            mpu_offsets.gyro_z += (int16_t)((buffer[12] << 8) | buffer[13]);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    mpu_offsets.accel_x /= num_readings;
    mpu_offsets.accel_y /= num_readings;
    mpu_offsets.accel_z /= num_readings;
    mpu_offsets.gyro_x /= num_readings;
    mpu_offsets.gyro_y /= num_readings;
    mpu_offsets.gyro_z /= num_readings;

    // The Z-axis accelerometer should read +1g (or ACCEL_SENSITIVITY) when level.
    // We need to subtract this ideal value to get the true offset.
    mpu_offsets.accel_z -= ACCEL_SENSITIVITY;

    ESP_LOGI(TAG, "Calibration complete. Offsets: AX:%ld, AY:%ld, AZ:%ld, GX:%ld, GY:%ld, GZ:%ld",
             mpu_offsets.accel_x, mpu_offsets.accel_y, mpu_offsets.accel_z,
             mpu_offsets.gyro_x, mpu_offsets.gyro_y, mpu_offsets.gyro_z);
}

// --- Private Helper Functions ---
static uint8_t shtc3_crc8(const uint8_t *data, int len)
{
    uint8_t crc = CRC8_INIT;
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// --- Main Init Function ---
void sensor_manager_init(void)
{
    // Create I2C bus
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    // Create a generic device config
    i2c_device_config_t dev_config = {
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    // Add MPU6050
    dev_config.device_address = MPU6050_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &mpu6050_handle));
    uint8_t mpu_wake_cmd[] = {MPU6050_PWR_MGMT_1, 0};
    ESP_ERROR_CHECK(i2c_master_transmit(mpu6050_handle, mpu_wake_cmd, sizeof(mpu_wake_cmd), -1));
    sensor_manager_calibrate_mpu6050();

    // // Add INA226 and Calibrate
    // dev_config.device_address = INA226_DEVICE_ADDRESS;
    // ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &ina226_handle));
    // ina226_current_lsb = MAX_EXPECTED_CURRENT_AMPS / 32768.0;
    // uint16_t cal = (uint16_t)(0.00512 / (ina226_current_lsb * SHUNT_RESISTANCE_OHMS));
    // uint8_t ina_cal_cmd[3] = {INA226_REG_CALIBRATION, (cal >> 8) & 0xFF, cal & 0xFF};
    // ESP_ERROR_CHECK(i2c_master_transmit(ina226_handle, ina_cal_cmd, sizeof(ina_cal_cmd), -1));
    // uint8_t ina_config_cmd[3] = {INA226_REG_CONFIG, (INA226_DEFAULT_CONFIG >> 8) & 0xFF, INA226_DEFAULT_CONFIG & 0xFF};
    // ESP_ERROR_CHECK(i2c_master_transmit(ina226_handle, ina_config_cmd, sizeof(ina_config_cmd), -1));

    // // Add SHTC3
    // dev_config.device_address = SHTC3_SENSOR_ADDR;
    // ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &shtc3_handle));

    ESP_LOGI(TAG, "All sensors initialized!");
}

// --- Separate Reading Functions ---

esp_err_t sensor_manager_read_ina226(ina226_data_t *data)
{
    if (!ina226_handle || !data)
        return ESP_ERR_INVALID_ARG;

    uint8_t read_buf[2];
    esp_err_t err;

    // Read Bus Voltage
    uint8_t vbus_reg = INA226_REG_BUSVOLTAGE;
    err = i2c_master_transmit_receive(ina226_handle, &vbus_reg, 1, read_buf, 2, -1);
    if (err != ESP_OK)
        return err;
    data->bus_voltage_v = ((read_buf[0] << 8) | read_buf[1]) * 1.25 / 1000.0;

    // Read Current
    uint8_t curr_reg = INA226_REG_CURRENT;
    err = i2c_master_transmit_receive(ina226_handle, &curr_reg, 1, read_buf, 2, -1);
    if (err != ESP_OK)
        return err;
    data->current_a = ((int16_t)((read_buf[0] << 8) | read_buf[1])) * ina226_current_lsb;

    return ESP_OK;
}

esp_err_t sensor_manager_read_shtc3(shtc3_data_t *data)
{
    if (!shtc3_handle || !data)
        return ESP_ERR_INVALID_ARG;

    uint8_t command_bytes[2];
    esp_err_t ret;

    // 1. Wakeup sensor
    command_bytes[0] = (SHTC3_CMD_WAKEUP >> 8) & 0xFF;
    command_bytes[1] = SHTC3_CMD_WAKEUP & 0xFF;
    ret = i2c_master_transmit(shtc3_handle, command_bytes, 2, -1);
    if (ret != ESP_OK)
        return ret;
    vTaskDelay(pdMS_TO_TICKS(1));

    // 2. Send measurement command
    command_bytes[0] = (SHTC3_CMD_MEASURE_T_FIRST >> 8) & 0xFF;
    command_bytes[1] = SHTC3_CMD_MEASURE_T_FIRST & 0xFF;
    ret = i2c_master_transmit(shtc3_handle, command_bytes, 2, -1);
    if (ret != ESP_OK)
        return ret;
    vTaskDelay(pdMS_TO_TICKS(20)); // Wait for measurement

    // 3. Read data
    uint8_t read_buffer[6];
    ret = i2c_master_receive(shtc3_handle, read_buffer, 6, -1);

    // 4. Send sleep command
    command_bytes[0] = (SHTC3_CMD_SLEEP >> 8) & 0xFF;
    command_bytes[1] = SHTC3_CMD_SLEEP & 0xFF;
    i2c_master_transmit(shtc3_handle, command_bytes, 2, -1);

    if (ret != ESP_OK)
        return ret;

    // 5. Verify CRC
    if (shtc3_crc8(read_buffer, 2) != read_buffer[2] || shtc3_crc8(read_buffer + 3, 2) != read_buffer[5])
    {
        return ESP_ERR_INVALID_CRC;
    }

    // 6. Convert raw data
    uint16_t raw_temp = (read_buffer[3] << 8) | read_buffer[4]; // T is second on Read T First
    uint16_t raw_rh = (read_buffer[0] << 8) | read_buffer[1];   // RH is first on Read T first
    data->temperature_c = -45.0f + 175.0f * (float)raw_temp / 65536.0f;
    data->humidity_rh = 100.0f * (float)raw_rh / 65536.0f;

    return ESP_OK;
}

esp_err_t sensor_manager_read_mpu6050(mpu6050_data_t *data)
{
    if (!mpu6050_handle || !data)
        return ESP_ERR_INVALID_ARG;

    uint8_t reg = MPU6050_ACCEL_XOUT_H;
    uint8_t buffer[14];
    esp_err_t err = i2c_master_transmit_receive(mpu6050_handle, &reg, 1, buffer, 14, -1);
    if (err != ESP_OK)
        return err;

    // Get RAW 16-bit integer values

    // int16_t accel_x_raw = ((buffer[0] << 8) | buffer[1]) - mpu_offsets.accel_x;
    // int16_t accel_y_raw = ((buffer[2] << 8) | buffer[3]) - mpu_offsets.accel_y;
    // int16_t accel_z_raw = ((buffer[4] << 8) | buffer[5]) - mpu_offsets.accel_z;
    // int16_t gyro_x_raw = ((buffer[8] << 8) | buffer[9]) - mpu_offsets.gyro_x;
    // int16_t gyro_y_raw = ((buffer[10] << 8) | buffer[11]) - mpu_offsets.gyro_y;
    // int16_t gyro_z_raw = ((buffer[12] << 8) | buffer[13]) - mpu_offsets.gyro_z;
    
    int16_t accel_x_raw = ((buffer[0] << 8) | buffer[1]);
    int16_t accel_y_raw = ((buffer[2] << 8) | buffer[3]);
    int16_t accel_z_raw = ((buffer[4] << 8) | buffer[5]);
    int16_t gyro_x_raw = ((buffer[8] << 8) | buffer[9]);
    int16_t gyro_y_raw = ((buffer[10] << 8) | buffer[11]);
    int16_t gyro_z_raw = ((buffer[12] << 8) | buffer[13]);

    // Convert accelerometer values to 'g' (gravity)
    data->accel_x_g = accel_x_raw / ACCEL_SENSITIVITY;
    data->accel_y_g = accel_y_raw / ACCEL_SENSITIVITY;
    data->accel_z_g = accel_z_raw / ACCEL_SENSITIVITY;

    // Convert gyroscope values to 'degrees per second'
    data->gyro_x_dps = gyro_x_raw / GYRO_SENSITIVITY;
    data->gyro_y_dps = gyro_y_raw / GYRO_SENSITIVITY;
    data->gyro_z_dps = gyro_z_raw / GYRO_SENSITIVITY;

    // Calculate Pitch and Roll from accelerometer data
    data->roll = atan2(data->accel_y_g, data->accel_z_g) * RAD_TO_DEG;
    data->pitch = atan2(-data->accel_x_g, sqrt(pow(data->accel_y_g, 2) + pow(data->accel_z_g, 2))) * RAD_TO_DEG;

    return ESP_OK;
}