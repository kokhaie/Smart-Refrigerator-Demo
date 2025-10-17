#include "sensor_manager.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <math.h>
#include <string.h>

static const char *TAG = "SENSOR_MANAGER";

// ------------------------- MPU REGISTERS -------------------------
#define MPU_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_USER_CTRL 0x6A
#define REG_FIFO_EN 0x23
#define REG_INT_ENABLE 0x38
#define REG_SMPLRT_DIV 0x19
#define REG_CONFIG 0x1A
#define REG_ACCEL_CONFIG 0x1C
#define REG_FIFO_COUNTH 0x72
#define REG_FIFO_COUNTL 0x73
#define REG_FIFO_R_W 0x74

#define USER_CTRL_FIFO_RESET 0x04
#define USER_CTRL_FIFO_EN 0x40
#define FIFO_EN_ACCEL 0x08
#define INT_EN_DATA_RDY 0x01
#define INT_EN_FIFO_OFLOW 0x10

#define FIFO_FRAME_SIZE 6
#define SAMPLE_RATE_HZ 1000
#define MPU_INT_PIN GPIO_NUM_21

// ------------------------- INA226 -------------------------
#define INA226_DEVICE_ADDRESS 0x40
#define INA226_REG_CONFIG 0x00
#define INA226_REG_CURRENT 0x04
#define INA226_REG_CALIB 0x05
#define INA226_DEFAULT_CONFIG 0x4127

// ------------------------- SHTC3 -------------------------
#define SHTC3_ADDR 0x70
#define SHTC3_CMD_WAKEUP 0x3517
#define SHTC3_CMD_MEASURE_TF 0x7CA2
#define CRC8_POLY 0x31
#define CRC8_INIT 0xFF

// ------------------------- Globals -------------------------
static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t mpu_dev, ina_dev, shtc_dev;

static SemaphoreHandle_t i2c_mutex;
static SemaphoreHandle_t data_mutex;
static SemaphoreHandle_t mpu_sem;

static float ina226_current_lsb;
static volatile ina226_data_t latest_ina = {0};
static volatile shtc3_data_t latest_shtc = {0};

// ------------------------- Queues -------------------------
#define STREAM_QUEUE_LEN 5 // for MQTT/real-time graph
#define BATCH_QUEUE_LEN 4   // number of batches buffered

static QueueHandle_t stream_queue;
static QueueHandle_t batch_queue;

static synchronized_sample_t batch_buf[BATCH_SIZE];
static int batch_index = 0;
static int downsample_counter = 0;

// ------------------------- I2C Helpers -------------------------
static esp_err_t i2c_write(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val)
{
    esp_err_t ret = ESP_FAIL;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)))
    {
        uint8_t buf[2] = {reg, val};
        ret = i2c_master_transmit(dev, buf, 2, -1);
        xSemaphoreGive(i2c_mutex);
    }
    return ret;
}

static esp_err_t i2c_read(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *data, size_t len)
{
    esp_err_t ret = ESP_FAIL;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)))
    {
        ret = i2c_master_transmit_receive(dev, &reg, 1, data, len, -1);
        xSemaphoreGive(i2c_mutex);
    }
    return ret;
}

// ------------------------- MPU Functions -------------------------
static void mpu_reset_fifo(void)
{
    ESP_LOGW(TAG, "Resetting FIFO");
    i2c_write(mpu_dev, REG_USER_CTRL, 0x00);
    vTaskDelay(pdMS_TO_TICKS(10));
    i2c_write(mpu_dev, REG_USER_CTRL, USER_CTRL_FIFO_RESET);
    vTaskDelay(pdMS_TO_TICKS(10));
    i2c_write(mpu_dev, REG_USER_CTRL, USER_CTRL_FIFO_EN);
    i2c_write(mpu_dev, REG_FIFO_EN, FIFO_EN_ACCEL);
}

static void mpu_init(void)
{
    ESP_LOGI(TAG, "Initializing MPU...");
    i2c_write(mpu_dev, REG_PWR_MGMT_1, 0x00);
    vTaskDelay(pdMS_TO_TICKS(100));
    i2c_write(mpu_dev, REG_SMPLRT_DIV, 0x00);
    i2c_write(mpu_dev, REG_CONFIG, 0x03);
    i2c_write(mpu_dev, REG_ACCEL_CONFIG, 0x00);
    mpu_reset_fifo();
    i2c_write(mpu_dev, REG_INT_ENABLE, INT_EN_DATA_RDY | INT_EN_FIFO_OFLOW);
}

// ISR
static void IRAM_ATTR mpu_isr(void *arg)
{
    BaseType_t hp = pdFALSE;
    xSemaphoreGiveFromISR(mpu_sem, &hp);
    if (hp)
        portYIELD_FROM_ISR();
}

// ------------------------- MPU Task -------------------------
static void mpu_task(void *arg)
{
    uint8_t buf[FIFO_FRAME_SIZE];
    for (;;)
    {
        if (xSemaphoreTake(mpu_sem, portMAX_DELAY))
        {
            uint8_t cnt_buf[2];
            uint16_t fifo_count = 0;
            if (i2c_read(mpu_dev, REG_FIFO_COUNTH, cnt_buf, 2) == ESP_OK)
                fifo_count = (cnt_buf[0] << 8) | cnt_buf[1];

            while (fifo_count >= FIFO_FRAME_SIZE)
            {
                if (i2c_read(mpu_dev, REG_FIFO_R_W, buf, FIFO_FRAME_SIZE) != ESP_OK)
                    break;
                fifo_count -= FIFO_FRAME_SIZE;

                int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
                int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
                int16_t az = (int16_t)((buf[4] << 8) | buf[5]);

                synchronized_sample_t pkt;
                pkt.timestamp_us = esp_timer_get_time();
                pkt.accel_x_g = ax / 16384.0f;
                pkt.accel_y_g = ay / 16384.0f;
                pkt.accel_z_g = az / 16384.0f;
                pkt.magnitude = sqrtf(pkt.accel_x_g * pkt.accel_x_g +
                                      pkt.accel_y_g * pkt.accel_y_g +
                                      pkt.accel_z_g * pkt.accel_z_g);

                if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)))
                {
                    pkt.latest_current_a = latest_ina.current_a;
                    pkt.latest_temperature_c = latest_shtc.temperature_c;
                    xSemaphoreGive(data_mutex);
                }

                // Push to real-time queue
                if (++downsample_counter >= 200)
                { // 1000/100 = 10 Hz
                    downsample_counter = 0;

                    if (xQueueSend(stream_queue, &pkt, 0) != pdPASS)
                    {
                        ESP_LOGI(TAG, "Stream queue full, replaceing with oldest");
                        // Queue full → drop oldest
                        synchronized_sample_t dummy;
                        xQueueReceive(stream_queue, &dummy, 0); // remove one
                        xQueueSend(stream_queue, &pkt, 0);      // now push latest
                    }
                }

                // // Fill batch
                // batch_buf[batch_index++] = pkt;
                // if (batch_index == BATCH_SIZE)
                // {
                //     if (xQueueSend(batch_queue, batch_buf, 0) != pdPASS)
                //     {
                //         ESP_LOGW(TAG, "Batch queue full, dropping batch");
                //     }
                //     batch_index = 0;
                // }
            }
        }
    }
}

// ------------------------- INA226 -------------------------
static esp_err_t read_ina226(ina226_data_t *out)
{
    uint8_t buf[2], reg = INA226_REG_CURRENT;
    if (i2c_read(ina_dev, reg, buf, 2) != ESP_OK)
        return ESP_FAIL;
    out->current_a = ((int16_t)((buf[0] << 8) | buf[1])) * ina226_current_lsb;
    return ESP_OK;
}

static void ina_task(void *arg)
{
    ina226_data_t tmp;
    for (;;)
    {
        if (read_ina226(&tmp) == ESP_OK)
        {
            if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(20)))
            {
                latest_ina = tmp;
                xSemaphoreGive(data_mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ------------------------- SHTC3 -------------------------
static uint8_t crc8(const uint8_t *data, int len)
{
    uint8_t crc = CRC8_INIT;
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x80) ? (crc << 1) ^ CRC8_POLY : (crc << 1);
    }
    return crc;
}

static esp_err_t shtc3_send_cmd(uint16_t cmd)
{
    uint8_t buf[2] = {(cmd >> 8) & 0xFF, cmd & 0xFF};
    esp_err_t ret = ESP_FAIL;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)))
    {
        ret = i2c_master_transmit(shtc_dev, buf, 2, -1);
        xSemaphoreGive(i2c_mutex);
    }
    return ret;
}

static esp_err_t read_shtc3(shtc3_data_t *out)
{
    uint8_t buf[6];
    if (shtc3_send_cmd(SHTC3_CMD_MEASURE_TF) != ESP_OK)
        return ESP_FAIL;

    vTaskDelay(pdMS_TO_TICKS(25));
    esp_err_t ret = ESP_FAIL;
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)))
    {
        ret = i2c_master_receive(shtc_dev, buf, 6, -1);
        xSemaphoreGive(i2c_mutex);
    }
    if (ret != ESP_OK)
        return ret;

    if (crc8(buf, 2) != buf[2] || crc8(buf + 3, 2) != buf[5])
        return ESP_ERR_INVALID_CRC;

    uint16_t raw_t = (buf[3] << 8) | buf[4];
    uint16_t raw_h = (buf[0] << 8) | buf[1];
    out->temperature_c = -45.0f + 175.0f * ((float)raw_t / 65536.0f);
    out->humidity_rh = 100.0f * ((float)raw_h / 65536.0f);

    return ESP_OK;
}

static void shtc_task(void *arg)
{
    shtc3_data_t tmp;
    for (;;)
    {
        if (read_shtc3(&tmp) == ESP_OK)
        {
            if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(20)))
            {
                latest_shtc = tmp;
                xSemaphoreGive(data_mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ------------------------- Public API -------------------------
bool sensor_manager_get_next_sample(synchronized_sample_t *out, TickType_t timeout)
{
    return (xQueueReceive(stream_queue, out, timeout) == pdTRUE);
}

bool sensor_manager_get_batch(synchronized_sample_t *out_batch, int *out_count, TickType_t timeout)
{
    if (xQueueReceive(batch_queue, out_batch, timeout) == pdTRUE)
    {
        *out_count = BATCH_SIZE;
        return true;
    }
    return false;
}

bool sensor_manager_get_latest_environment(shtc3_data_t *out)
{
    if (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)))
    {
        *out = latest_shtc;
        xSemaphoreGive(data_mutex);
        return true;
    }
    return false;
}

esp_err_t sensor_manager_init(void)
{
    // I2C bus
    i2c_master_bus_config_t cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = 15,
        .scl_io_num = 16,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false};
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .scl_speed_hz = 400000};

    // OS objects
    i2c_mutex = xSemaphoreCreateMutex();
    data_mutex = xSemaphoreCreateMutex();
    mpu_sem = xSemaphoreCreateBinary();

    stream_queue = xQueueCreate(STREAM_QUEUE_LEN, sizeof(synchronized_sample_t));
    // batch_queue = xQueueCreate(BATCH_QUEUE_LEN, sizeof(synchronized_sample_t) * BATCH_SIZE);
    batch_queue = xQueueCreate(BATCH_QUEUE_LEN, sizeof(synchronized_sample_t *));

    if (!i2c_mutex || !data_mutex || !mpu_sem || !stream_queue || !batch_queue)
    {
        ESP_LOGE(TAG, "Failed to allocate RTOS objects (queue/semaphore creation failed)");
        return ESP_ERR_NO_MEM;
    }

    // MPU
    dev_cfg.device_address = MPU_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &mpu_dev));
    mpu_init();

    // INA
    dev_cfg.device_address = INA226_DEVICE_ADDRESS;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &ina_dev));
    float max_current = 5.0f, shunt = 0.1f;
    ina226_current_lsb = max_current / 32768.0f;
    uint16_t cal = (uint16_t)(0.00512 / (ina226_current_lsb * shunt));
    uint8_t cal_cmd[3] = {INA226_REG_CALIB, (cal >> 8) & 0xFF, cal & 0xFF};
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)))
    {
        i2c_master_transmit(ina_dev, cal_cmd, 3, -1);
        uint8_t cfg_cmd[3] = {INA226_REG_CONFIG, (INA226_DEFAULT_CONFIG >> 8) & 0xFF,
                              INA226_DEFAULT_CONFIG & 0xFF};
        i2c_master_transmit(ina_dev, cfg_cmd, 3, -1);
        xSemaphoreGive(i2c_mutex);
    }

    // SHTC3
    dev_cfg.device_address = SHTC3_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &shtc_dev));
    uint8_t wake_cmd[2] = {(SHTC3_CMD_WAKEUP >> 8) & 0xFF, SHTC3_CMD_WAKEUP & 0xFF};
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)))
    {
        i2c_master_transmit(shtc_dev, wake_cmd, 2, -1);
        xSemaphoreGive(i2c_mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // Tasks
    xTaskCreatePinnedToCore(mpu_task, "mpu_task", 6144, NULL, 12, NULL, 1);
    xTaskCreatePinnedToCore(ina_task, "ina_task", 2048, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore(shtc_task, "shtc_task", 4096, NULL, 5, NULL, 1);

    // GPIO INT
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << MPU_INT_PIN),
        .pull_up_en = 0};
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(MPU_INT_PIN, mpu_isr, NULL);

    ESP_LOGI(TAG, "Sensor manager initialized.");
    return ESP_OK;
}
