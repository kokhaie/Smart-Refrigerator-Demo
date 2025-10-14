#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "business_logic.h"
#include "touch_slider.h"
#include "motors.h"
#include "data_publisher.h"
#include "ui_manager.h"

static const char *TAG = "MAIN_APP";

void sensor_manager_task(void *pvParameters)
{
    ina226_data_t power_data;
    mpu6050_data_t imu_data;
    shtc3_data_t temp_data;
    g_setpoint_queue = xQueueCreate(1, sizeof(float));

    for (;;)
    {
        // if (sensor_manager_read_ina226(&power_data) == ESP_OK)
        // {
        //     ESP_LOGI(TAG, "INA226 -> Voltage: %.2f V, Current: %.3f A", power_data.bus_voltage_v, power_data.current_a);
        //     publish_power_data(&power_data);
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "Failed to read INA226");
        // }

        // if (sensor_manager_read_shtc3(&temp_data) == ESP_OK)
        // {
        //     ESP_LOGI(TAG, "SHTC3  -> Temp: %.2f C, Humidity: %.1f %%", temp_data.temperature_c, temp_data.humidity_rh);
        //     update_setpoint(temp_data.temperature_c);
        //     publish_environment_data(&temp_data);
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "Failed to read SHTC3");
        // }

        if (sensor_manager_read_mpu6050(&imu_data) == ESP_OK)
        {
            ESP_LOGI(TAG, "MPU6050 -> Accel(g) X:%.2f Y:%.2f Z:%.2f | Gyro(dps) X:%.2f Y:%.2f Z:%.2f | Pitch:%.1f Roll:%.1f",
                     imu_data.accel_x_g, imu_data.accel_y_g, imu_data.accel_z_g,
                     imu_data.gyro_x_dps, imu_data.gyro_y_dps, imu_data.gyro_z_dps,
                     imu_data.pitch, imu_data.roll);
            publish_motion_data(&imu_data);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read MPU6050");
        }
        vTaskDelay(pdTICKS_TO_MS(100));
    }
}
void touch_slider_task(void *pvParameters)
{
    uint8_t last_position = 25;

    for (;;)
    {
        // --- Handle Events ---
        // First, check if the specific double-touch event occurred.
        if (touch_slider_was_double_touched())
        {
            ESP_LOGW(TAG, "ACTION: Double Touch Event Triggered!");
            // Add your double-touch action here (e.g., toggle a light)
        }

        // --- Handle State ---
        // Separately, get the current slider position for continuous control.
        uint8_t current_position = (uint8_t)touch_slider_get_position();

        // Only log if the position has changed to prevent flooding the console
        if (current_position != last_position)
        {
            ESP_LOGI(TAG, "Slider Position: %" PRIu32, current_position);
            last_position = current_position;
            publish_slider_setpoint(last_position);
            // Add your slider action here (e.g., set brightness)
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // Poll for events and state at 50Hz
    }
}
void ui_manager_task(void *pvParameters) {}
void network_manager_task(void *pvParameters) {}
void business_logic_task(void *pvParameters) {}
void anomaly_detection_task(void *pvParameters) {}
void start_tasks(void)
{
    /*
     * =================================================================
     * ESP32-S3 R8N8 Dual-Core Task Assignment Plan
     * =================================================================
     *
     * -----------------------------------------------------------------
     * Core 0: Protocol, UI, and Non-Real-Time Tasks
     * -----------------------------------------------------------------
     * Responsible for tasks that are I/O bound (wait for network/user)
     * or are less time-sensitive. The Wi-Fi stack runs here by default.
     *
     * Components on Core 0:
     * - Wi-Fi Handler: Manages Wi-Fi connection and events.
     * - MQTT Client: Handles all MQTT communication (pub/sub).
     * - Touch Sensor Input: Reads user input from the touch slider.
     * - LCD Display Controller: Updates the screen (UI rendering).
     * - User Behavior Model: Low-priority, CPU-intensive background task.
     *
     *
     * -----------------------------------------------------------------
     * Core 1: Real-Time Control and High-Speed Processing
     * -----------------------------------------------------------------
     * Dedicated to time-critical tasks that require consistent,
     * low-latency execution to ensure system stability and safety.
     *
     * Components on Core 1:
     * - Motor Controller (PID Loop): Highest priority for stable control.
     * - I2C Sensors Reader: High priority to feed data with low delay.
     * - Vibration Anomaly Model: Medium priority, processes sensor data
     * without interfering with the primary motor control loop.
     *
     * =================================================================
     */

    // xTaskCreatePinnedToCore(touch_slider_task, "touch_slider_task", 4096, NULL, 12, NULL, 0);
    // xTaskCreatePinnedToCore(network_manager_task, "network_manager_task", 4096, NULL, 10, NULL, 0);
    // xTaskCreatePinnedToCore(ui_manager_task, "ui_manager_task", 4096, NULL, 8, NULL, 0);

    xTaskCreatePinnedToCore(sensor_manager_task, "sensor_manager_task", 4096, NULL, 12, NULL, 1);
    // xTaskCreatePinnedToCore(business_logic_task, "business_logic_task", 4096, NULL, 10, NULL, 1);
    // xTaskCreatePinnedToCore(anomaly_detection_task, "anomaly_detection", 4096, NULL, 8, NULL, 1);
}
void app_status_update_cb(network_status_t status)
{
    // Here you would update your LCD based on the status
    switch (status)
    {
    case NETWORK_STATUS_INITIALIZING:
        ESP_LOGI(TAG, "LCD UPDATE: Initializing...");
        // lcd_display("Initializing...");
        break;
    case NETWORK_STATUS_CONNECTING_WIFI:
        ESP_LOGI(TAG, "LCD UPDATE: Connecting Wi-Fi...");
        // lcd_display("Connecting WiFi");
        break;
    case NETWORK_STATUS_CONNECTING_MQTT:
        ESP_LOGI(TAG, "LCD UPDATE: Connecting Broker...");
        // lcd_display("Connecting Broker");
        break;
    case NETWORK_STATUS_CONNECTED_INTERNET:
        ESP_LOGI(TAG, "LCD UPDATE: Connected!");
        start_tasks();
        // mqtt connected
        // lcd_display("Online!");

        break;
    case NETWORK_STATUS_CONNECTION_FAILED:
        ESP_LOGI(TAG, "LCD UPDATE: Connection Failed.");
        // lcd_display("Connection Failed");
        break;
    case NETWORK_STATUS_STARTING_AP_MODE:
        ESP_LOGI(TAG, "LCD UPDATE: Starting AP Mode...");
        // lcd_display("Starting AP Mode");
        break;
    case NETWORK_STATUS_STARTING_LOCAL_BROKER:
        ESP_LOGI(TAG, "LCD UPDATE: Starting Broker...");
        // lcd_display("Starting Broker");
        break;
    case NETWORK_STATUS_AP_MODE_ACTIVE:
        ESP_LOGI(TAG, "LCD UPDATE: AP Active at 192.168.4.1");
        // lcd_display("AP Active:\n192.168.4.1");
        break;
    }
}
void app_main(void)
{
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);

    // // touch_slider_init();
    // // motors_init();

    // //  ui manager init

    // if (sensor_manager_init() != ESP_OK)
    //     ESP_LOGE("MAIN", "Sensor initialization failed. Halting application.");
    // network_manager_start(app_status_update_cb);
    ui_manager_start();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
