#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sensor_manager.h"
#include "touch_slider.h"

static const char *TAG = "MAIN_APP";
void main_app_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Main application task started.");
    uint32_t last_position = 999; // Store last position to avoid spamming the log

    while (1)
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
        uint32_t current_position = touch_slider_get_position();

        // Only log if the position has changed to prevent flooding the console
        if (current_position != last_position)
        {
            ESP_LOGI(TAG, "Slider Position: %" PRIu32, current_position);
            last_position = current_position;
            // Add your slider action here (e.g., set brightness)
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // Poll for events and state at 50Hz
    }
}

void app_main(void)
{
    // // Initialize all sensors with one function call
    // sensor_manager_init();

    // // Create structs to hold the data for each sensor
    // ina226_data_t power_data;
    // shtc3_data_t env_data;
    // mpu6050_data_t imu_data;
    touch_slider_init();

    // // Read each sensor individually
    // if (sensor_manager_read_ina226(&power_data) == ESP_OK) {
    //     ESP_LOGI(TAG, "INA226 -> Voltage: %.2f V, Current: %.3f A", power_data.bus_voltage_v, power_data.current_a);
    // } else {
    //     ESP_LOGE(TAG, "Failed to read INA226");
    // }

    // if (sensor_manager_read_shtc3(&env_data) == ESP_OK) {
    //     ESP_LOGI(TAG, "SHTC3  -> Temp: %.2f C, Humidity: %.1f %%", env_data.temperature_c, env_data.humidity_rh);
    // } else {
    //     ESP_LOGE(TAG, "Failed to read SHTC3");
    // }

    // if (sensor_manager_read_mpu6050(&imu_data) == ESP_OK)
    // {
    //     ESP_LOGI(TAG, "MPU6050 -> Accel(g) X:%.2f Y:%.2f Z:%.2f | Gyro(dps) X:%.2f Y:%.2f Z:%.2f | Pitch:%.1f Roll:%.1f",
    //              imu_data.accel_x_g, imu_data.accel_y_g, imu_data.accel_z_g,
    //              imu_data.gyro_x_dps, imu_data.gyro_y_dps, imu_data.gyro_z_dps,
    //              imu_data.pitch, imu_data.roll);
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "Failed to read MPU6050");
    // }

    xTaskCreate(main_app_task, "main_app_task", 4096, NULL, 5, NULL);
    // vTaskDelay(pdMS_TO_TICKS(500));
}