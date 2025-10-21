#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "business_logic.h"
#include "touch_slider.h"
#include "motor_manager.h"
#include "data_publisher.h"
#include "led_manager.h"
#include "ui_controller.h"
#include "lcd_manager.h"
#include "esp_sleep.h"

static const char *TAG = "MAIN_APP";

void app_status_update_cb(network_status_t status)
{
    // Here you would update your LCD based on the status
    switch (status)
    {
    case NETWORK_STATUS_INITIALIZING:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Initializing...");
        // lcd_display("Initializing...");
        break;
    case NETWORK_STATUS_CONNECTING_WIFI:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Connecting Wi-Fi...");
        // lcd_display("Connecting WiFi");
        break;
    case NETWORK_STATUS_CONNECTING_MQTT:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Connecting Broker...");
        // lcd_display("Connecting Broker");
        break;
    case NETWORK_STATUS_CONNECTED_INTERNET:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_ONLINE);
        // ESP_LOGI(TAG, "LCD UPDATE: Connected!");
        // mqtt connected

        data_publisher_start();
        // lcd_display("Online!");

        break;
    case NETWORK_STATUS_CONNECTION_FAILED:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_OFFLINE);
        ESP_LOGI(TAG, "LCD UPDATE: Connection Failed.");
        // lcd_display("Connection Failed");
        break;
    case NETWORK_STATUS_STARTING_AP_MODE:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Starting AP Mode...");
        // lcd_display("Starting AP Mode");
        break;
    case NETWORK_STATUS_STARTING_LOCAL_BROKER:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Starting Broker...");
        // lcd_display("Starting Broker");
        break;
    case NETWORK_STATUS_AP_MODE_ACTIVE:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: AP Active at 192.168.4.1");
        // lcd_display("AP Active:\n192.168.4.1");
        break;
    }
}

void power_off_system(void)
{
    ESP_LOGI(TAG, "Powering off system...");

    // Drive actuators and indicators to a safe state before shutting down.
    set_fan_speed(0);
    set_vibration_speed(0);
    led_manager_stop_rainbow();
    led_manager_clear();
    lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_OFFLINE);

    ui_controller_cleanup();

    // Give peripherals a brief moment to settle before entering deep sleep.
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "Entering deep sleep");
    esp_deep_sleep_start();
}
void app_main(void)
{
    // Init NVS for WiFi / MQTT credentials
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_ERROR_CHECK(ret);

    // Init sensor manager
    if (sensor_manager_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensor initialization failed. Halting.");
        return;
    }

    motor_manager_init();
    business_logic_start();
    touch_slider_init();

    lcd_manager_start();
    lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_OFFLINE);

    // // Start network manager (Wi-Fi + MQTT)
    network_manager_start(app_status_update_cb);
    led_manager_init();
    led_manager_set_global_brightness(0.25f);
    ESP_ERROR_CHECK(ui_controller_init());

    // Logger is independent of network → always available
    // data_logger_start();
    // power_off_system();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
