#include "sensor_manager.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <math.h>

static const char *TAG = "SENSOR_MANAGER";

// --- Private Handles and State Variables ---
static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t mpu6050_handle;
static i2c_master_dev_handle_t ina226_handle;
static i2c_master_dev_handle_t shtc3_handle;
static float ina226_current_lsb; // Stores the calculated LSB for INA226

// Mutex to protect the shared I2C bus
static SemaphoreHandle_t i2c_mutex;
// Mutex to protect the global data structs during reads/writes
static SemaphoreHandle_t data_mutex;

// MPU6050 Defines
#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B
#define ACCEL_SENSITIVITY 16384.0
#define GYRO_SENSITIVITY 131.0
#define RAD_TO_DEG (180.0 / M_PI)

// MPU6050 Register Defines for FIFO
#define MPU6050_USER_CTRL 0x6A
#define MPU6050_FIFO_EN 0x23
#define MPU6050_FIFO_COUNTH 0x72
#define MPU6050_FIFO_R_W 0x74
#define MPU6050_SMPLRT_DIV 0x19

// INA226 Defines
#define INA226_DEVICE_ADDRESS 0x40
#define INA226_REG_CONFIG 0x00
#define INA226_REG_BUSVOLTAGE 0x02
#define INA226_REG_CURRENT 0x04
#define INA226_REG_CALIBRATION 0x05
#define INA226_DEFAULT_CONFIG 0x4127

// SHTC3 Defines
#define SHTC3_SENSOR_ADDR 0x70
#define SHTC3_CMD_WAKEUP 0x3517
#define SHTC3_CMD_SLEEP 0xB098
#define SHTC3_CMD_MEASURE_T_FIRST 0x7CA2
#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF

// --- Global Data Structures ---
// Use volatile to ensure the compiler doesn't optimize away reads
static volatile mpu6050_data_t latest_mpu_data = {0};
static volatile ina226_data_t latest_ina_data = {0};
static volatile shtc3_data_t latest_shtc3_data = {0};

// Circular buffer for vibration data
#define VIBRATION_BUFFER_SIZE 2048
volatile float vibration_buffer[VIBRATION_BUFFER_SIZE];
volatile int vibration_write_idx = 0;

static mpu6050_offsets_t mpu_offsets = {0};

// --- Private Helper Functions ---
static uint8_t shtc3_crc8(const uint8_t *data, int len)
{
    uint8_t crc = CRC8_INIT;
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ CRC8_POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}

static esp_err_t mpu6050_fifo_init(i2c_master_dev_handle_t dev_handle)
{
    uint8_t cmd_buffer[2];
    // Set Sample Rate to 1kHz
    cmd_buffer[0] = MPU6050_SMPLRT_DIV;
    cmd_buffer[1] = 7; // (8kHz Gyro / (1+7) = 1kHz)
    i2c_master_transmit(dev_handle, cmd_buffer, 2, pdMS_TO_TICKS(200));
    // Enable FIFO for Accelerometer data only
    cmd_buffer[0] = MPU6050_FIFO_EN;
    cmd_buffer[1] = 0x08; // ACCEL_FIFO_EN
    i2c_master_transmit(dev_handle, cmd_buffer, 2, pdMS_TO_TICKS(200));
    // Enable and Reset the FIFO
    cmd_buffer[0] = MPU6050_USER_CTRL;
    cmd_buffer[1] = 0x44; // FIFO_EN and FIFO_RESET
    i2c_master_transmit(dev_handle, cmd_buffer, 2, pdMS_TO_TICKS(200));
    return ESP_OK;
}

static esp_err_t do_read_ina226(ina226_data_t *data)
{
    uint8_t read_buf[2];
    esp_err_t err;
    uint8_t vbus_reg = INA226_REG_BUSVOLTAGE;
    err = i2c_master_transmit_receive(ina226_handle, &vbus_reg, 1, read_buf, 2, pdMS_TO_TICKS(200));
    if (err != ESP_OK)
        return err;
    data->bus_voltage_v = ((read_buf[0] << 8) | read_buf[1]) * 1.25 / 1000.0;

    uint8_t curr_reg = INA226_REG_CURRENT;
    err = i2c_master_transmit_receive(ina226_handle, &curr_reg, 1, read_buf, 2, pdMS_TO_TICKS(200));
    if (err != ESP_OK)
        return err;
    data->current_a = ((int16_t)((read_buf[0] << 8) | read_buf[1])) * ina226_current_lsb;
    return ESP_OK;
}

static esp_err_t do_read_shtc3(shtc3_data_t *data)
{
    esp_err_t ret;
    uint8_t cmd[2];
    uint8_t buffer[6];

    // 1. Wakeup
    cmd[0] = (SHTC3_CMD_WAKEUP >> 8) & 0xFF;
    cmd[1] = SHTC3_CMD_WAKEUP & 0xFF;
    ret = i2c_master_transmit(shtc3_handle, cmd, 2, pdMS_TO_TICKS(200));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SHTC3 Wakeup failed");
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(15));

    // 2. Measure
    cmd[0] = (SHTC3_CMD_MEASURE_T_FIRST >> 8) & 0xFF;
    cmd[1] = SHTC3_CMD_MEASURE_T_FIRST & 0xFF;
    ret = i2c_master_transmit(shtc3_handle, cmd, 2, pdMS_TO_TICKS(200));
    vTaskDelay(pdMS_TO_TICKS(15));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SHTC3 Measure command failed");
        // Attempt to sleep anyway to recover state
        cmd[0] = (SHTC3_CMD_SLEEP >> 8) & 0xFF;
        cmd[1] = SHTC3_CMD_SLEEP & 0xFF;
        i2c_master_transmit(shtc3_handle, cmd, 2, pdMS_TO_TICKS(200));
        vTaskDelay(pdMS_TO_TICKS(15));

        return ret;
    }
    // 3. Read
    ret = i2c_master_receive(shtc3_handle, buffer, 6, pdMS_TO_TICKS(200));

    // 4. Sleep (Always attempt to sleep to ensure a known state)
    cmd[0] = (SHTC3_CMD_SLEEP >> 8) & 0xFF;
    cmd[1] = SHTC3_CMD_SLEEP & 0xFF;
    i2c_master_transmit(shtc3_handle, cmd, 2, pdMS_TO_TICKS(200));
    vTaskDelay(pdMS_TO_TICKS(15));

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SHTC3 Read failed");
        return ret;
    }

    // 5. CRC Check
    if (shtc3_crc8(buffer, 2) != buffer[2] || shtc3_crc8(buffer + 3, 2) != buffer[5])
    {
        ESP_LOGE(TAG, "SHTC3 CRC failed");
        return ESP_ERR_INVALID_CRC;
    }

    // 6. Convert Data
    uint16_t raw_temp = (buffer[3] << 8) | buffer[4];
    uint16_t raw_rh = (buffer[0] << 8) | buffer[1];

    data->temperature_c = -45.0f + 175.0f * (float)raw_temp / 65536.0f;
    data->humidity_rh = 100.0f * (float)raw_rh / 65536.0f;

    return ESP_OK;
}

static void mpu_task(void *pvParameters)
{
    uint8_t fifo_count_reg = MPU6050_FIFO_COUNTH;
    uint8_t fifo_data_reg = MPU6050_FIFO_R_W;
    uint8_t count_buffer[2];
    uint8_t data_buffer[252]; // Read in chunks

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50);

    for (;;)
    {

        mpu6050_data_t current_mpu_data = {0};
        bool data_was_processed = false;

        // --- I2C Bus Critical Section ---
        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            // FIX: Add a FIFO reset before each read to clear any corrupted data
            uint8_t reset_cmd[] = {MPU6050_USER_CTRL, 0x44}; // FIFO_EN and FIFO_RESET
            i2c_master_transmit(mpu6050_handle, reset_cmd, 2, pdMS_TO_TICKS(200));

            uint16_t fifo_count = 0;
            if (i2c_master_transmit_receive(mpu6050_handle, &fifo_count_reg, 1, count_buffer, 2, pdMS_TO_TICKS(200)) == ESP_OK)
            {
                fifo_count = (count_buffer[0] << 8) | count_buffer[1];
            }

            if (fifo_count >= 6)
            { // At least one full sample
                int bytes_to_read = (fifo_count > sizeof(data_buffer)) ? sizeof(data_buffer) : fifo_count;
                bytes_to_read = (bytes_to_read / 6) * 6; // Read only full samples

                if (i2c_master_transmit_receive(mpu6050_handle, &fifo_data_reg, 1, data_buffer, bytes_to_read, pdMS_TO_TICKS(200)) == ESP_OK)
                {
                    // Process data into a local struct. No data_mutex needed here.
                    for (int i = 0; i < bytes_to_read; i += 6)
                    {
                        int16_t ax_raw = (int16_t)((data_buffer[i] << 8) | data_buffer[i + 1]);
                        int16_t ay_raw = (int16_t)((data_buffer[i + 2] << 8) | data_buffer[i + 3]);
                        int16_t az_raw = (int16_t)((data_buffer[i + 4] << 8) | data_buffer[i + 5]);

                        current_mpu_data.accel_x_g = (ax_raw - mpu_offsets.accel_x) / ACCEL_SENSITIVITY;
                        current_mpu_data.accel_y_g = (ay_raw - mpu_offsets.accel_y) / ACCEL_SENSITIVITY;
                        current_mpu_data.accel_z_g = (az_raw - mpu_offsets.accel_z) / ACCEL_SENSITIVITY;

                        float magnitude = sqrtf((float)ax_raw * ax_raw + (float)ay_raw * ay_raw + (float)az_raw * az_raw);
                        vibration_buffer[vibration_write_idx] = magnitude;
                        vibration_write_idx = (vibration_write_idx + 1) % VIBRATION_BUFFER_SIZE;
                        data_was_processed = true;
                    }
                }
            }
            xSemaphoreGive(i2c_mutex); // Release I2C bus
        }

        // --- Shared Data Critical Section ---
        if (data_was_processed)
        {
            if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
            {
                latest_mpu_data = current_mpu_data; // Quick copy
                xSemaphoreGive(data_mutex);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

static void ina_task(void *pvParameters)
{
    ina226_data_t temp_data;
    for (;;)
    {
        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            if (do_read_ina226(&temp_data) == ESP_OK)
            {
                if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
                {
                    latest_ina_data = temp_data;
                    xSemaphoreGive(data_mutex);
                }
            }
            xSemaphoreGive(i2c_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Run every 100ms
    }
}

static void shtc_task(void *pvParameters)
{
    shtc3_data_t temp_data;
    for (;;)
    {
        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
        {

            if (do_read_shtc3(&temp_data) == ESP_OK)
            {
                if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
                {
                    latest_shtc3_data = temp_data;
                    xSemaphoreGive(data_mutex);
                }
            }
            xSemaphoreGive(i2c_mutex);
        }
        else
        {
            // This will tell us if the task is failing to get the mutex
            ESP_LOGE(TAG, "SHTC3 task could not get I2C mutex lock.");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// These functions provide thread-safe access to the latest sensor data
esp_err_t sensor_manager_get_latest_mpu6050_data(mpu6050_data_t *data)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        *data = latest_mpu_data;
        xSemaphoreGive(data_mutex);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t sensor_manager_get_latest_ina226_data(ina226_data_t *data)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        *data = latest_ina_data;
        xSemaphoreGive(data_mutex);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t sensor_manager_get_latest_shtc3_data(shtc3_data_t *data)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        *data = latest_shtc3_data;
        xSemaphoreGive(data_mutex);
        return ESP_OK;
    }
    return ESP_FAIL;
}

// --- Main Init Function ---
esp_err_t sensor_manager_init(void)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = CONFIG_I2C_MASTER_SDA_IO,
        .scl_io_num = CONFIG_I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    i2c_device_config_t dev_config = {.scl_speed_hz = CONFIG_I2C_MASTER_FREQ_HZ};

    // Init MPU6050
    dev_config.device_address = MPU6050_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &mpu6050_handle));
    uint8_t mpu_wake_cmd[] = {MPU6050_PWR_MGMT_1, 0};
    ESP_ERROR_CHECK(i2c_master_transmit(mpu6050_handle, mpu_wake_cmd, sizeof(mpu_wake_cmd), -1));
    ESP_ERROR_CHECK(mpu6050_fifo_init(mpu6050_handle));

    // Init INA226
    dev_config.device_address = INA226_DEVICE_ADDRESS;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &ina226_handle));

    // --- Convert all values to base units (Amps and Ohms) before calculating ---

    // 1. Get your configured values from sdkconfig
    const float max_expected_current_mA = CONFIG_INA226_MAX_CURRENT_MILLIAMPS;    // This will be 500.0
    const float shunt_resistance_mOhm = CONFIG_INA226_SHUNT_RESISTANCE_MILLIOHMS; // This should be 100.0 for a 0.1 Ohm resistor

    // 2. Convert to Amps and Ohms
    float max_expected_current_A = max_expected_current_mA / 1000.0; // 500mA -> 0.5A
    float shunt_resistance_Ohm = shunt_resistance_mOhm / 1000.0;     // 100mOhm -> 0.1 Ohm

    // 3. Calculate Current_LSB in Amps per bit. This will be stored for later.
    ina226_current_lsb = max_expected_current_A / 32768.0;

    // 4. Calculate the calibration value using the formula with base units
    uint16_t cal = (uint16_t)(0.00512 / (ina226_current_lsb * shunt_resistance_Ohm));

    ESP_LOGI(TAG, "INA226: Max Current=%.2fA, Shunt=%.3fOhm, LSB=%.6fA/bit, CAL=%d",
             max_expected_current_A, shunt_resistance_Ohm, ina226_current_lsb, cal);

    // 5. Write the new calibration value to the sensor
    uint8_t ina_cal_cmd[3] = {INA226_REG_CALIBRATION, (cal >> 8) & 0xFF, cal & 0xFF};
    ESP_ERROR_CHECK(i2c_master_transmit(ina226_handle, ina_cal_cmd, sizeof(ina_cal_cmd), -1));

    // Write the configuration register
    uint8_t ina_config_cmd[3] = {INA226_REG_CONFIG, (INA226_DEFAULT_CONFIG >> 8) & 0xFF, INA226_DEFAULT_CONFIG & 0xFF};
    ESP_ERROR_CHECK(i2c_master_transmit(ina226_handle, ina_config_cmd, sizeof(ina_config_cmd), -1));

    // Init SHTC3
    dev_config.device_address = SHTC3_SENSOR_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &shtc3_handle));
    uint8_t shtc3_wake_cmd[] = {(SHTC3_CMD_WAKEUP >> 8) & 0xFF, SHTC3_CMD_WAKEUP & 0xFF};
    ESP_ERROR_CHECK(i2c_master_transmit(shtc3_handle, shtc3_wake_cmd, sizeof(shtc3_wake_cmd), -1));
    uint8_t shtc3_sleep_cmd[] = {(SHTC3_CMD_SLEEP >> 8) & 0xFF, SHTC3_CMD_SLEEP & 0xFF};
    ESP_ERROR_CHECK(i2c_master_transmit(shtc3_handle, shtc3_sleep_cmd, sizeof(shtc3_sleep_cmd), -1));

    ESP_LOGI(TAG, "All sensors initialized!");

    // Create Mutexes
    i2c_mutex = xSemaphoreCreateMutex();
    data_mutex = xSemaphoreCreateMutex();
    if (i2c_mutex == NULL || data_mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create mutexes");
        return ESP_FAIL;
    }

    // Create Tasks
    xTaskCreatePinnedToCore(mpu_task, "mpu_task", 2048, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(ina_task, "ina_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(shtc_task, "shtc_task", 2048, NULL, 4, NULL, 1);

    ESP_LOGI(TAG, "Sensor manager initialized and background tasks started.");
    return ESP_OK;
}