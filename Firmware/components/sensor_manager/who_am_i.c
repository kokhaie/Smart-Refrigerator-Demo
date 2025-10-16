#include "driver/i2c_master.h"
#include "esp_log.h"

#define SDA_PIN 15
#define SCL_PIN 16
#define MPU_ADDR 0x68
#define WHO_AM_I 0x75
#define ACCEL_XOUT_H 0x3B

// static const char *TAG = "MAIN_APP";

static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t mpu_dev;


static void i2c_init(void) {
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,   // you have external resistors
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &mpu_dev));
}

static void read_whoami(void) {
    uint8_t reg = WHO_AM_I;
    uint8_t val = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(mpu_dev, &reg, 1, &val, 1, 100));
    ESP_LOGI(TAG, "WHO_AM_I = 0x%02X", val);
}

static void read_accel_gyro(void) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t buf[14];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(mpu_dev, &reg, 1, buf, 14, 100));

    int16_t ax = (buf[0] << 8) | buf[1];
    int16_t ay = (buf[2] << 8) | buf[3];
    int16_t az = (buf[4] << 8) | buf[5];
    int16_t gx = (buf[8] << 8) | buf[9];
    int16_t gy = (buf[10] << 8) | buf[11];
    int16_t gz = (buf[12] << 8) | buf[13];

    ESP_LOGI(TAG, "Accel: [%d, %d, %d]  Gyro: [%d, %d, %d]",
             ax, ay, az, gx, gy, gz);
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing I2C...");
    i2c_init();

    ESP_LOGI(TAG, "Reading WHO_AM_I...");
    read_whoami();

    while (1) {
        read_accel_gyro();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}