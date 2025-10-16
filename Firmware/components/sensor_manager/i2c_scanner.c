#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#define I2C_SDA_PIN 15
#define I2C_SCL_PIN 16
#define I2C_CLK_HZ  100000   // 100kHz

static const char *TAG = "i2c_scanner";

void app_main(void)
{
    // Configure the I2C bus
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    ESP_LOGI(TAG, "Starting I2C scan...");

    uint8_t dummy = 0;  // 1-byte dummy buffer
    int found = 0;

    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = I2C_CLK_HZ,
        };

        i2c_master_dev_handle_t dev_handle;
        esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
        if (ret == ESP_OK) {
            // Try to write 1 dummy byte, ignore what it does
            ret = i2c_master_transmit(dev_handle, &dummy, 1, 10);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Found device at 0x%02X", addr);
                found++;
            }
            i2c_master_bus_rm_device(dev_handle);
        }
    }

    if (found == 0) {
        ESP_LOGI(TAG, "No I2C devices found.");
    } else {
        ESP_LOGI(TAG, "Total devices found: %d", found);
    }
}
