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

static const char *TAG = "MAIN_APP";

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
        // ESP_LOGI(TAG, "LCD UPDATE: Connected!");
        // mqtt connected

        data_publisher_start();
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
    // Init NVS for WiFi / MQTT credentials
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Init sensor manager
    if (sensor_manager_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensor initialization failed. Halting.");
        return;
    }


    // touch_slider_init();
    motors_init();
    business_logic_start();
    update_setpoint(25);

    //  ui manager init

    // Start network manager (Wi-Fi + MQTT)
    network_manager_start(app_status_update_cb);

    // Logger is independent of network → always available
    // data_logger_start();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
