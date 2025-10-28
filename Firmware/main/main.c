#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_random.h"
#include "nvs_flash.h"
#include <math.h>
#include <stdlib.h>

#include "anomaly_detector.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "business_logic.h"
#include "touch_slider.h"
#include "motor_manager.h"
#include "data_publisher.h"
#include "led_manager.h"
#include "ui_controller.h"
#include "lcd_manager.h"
#include "data_collector.h"
#include "fault_simulator.h"

static const char *TAG = "MAIN_APP";

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

static const char *MODEL_CLASS_LABELS[ANOMALY_MODEL_CLASS_COUNT] = {
    "NORMAL",
    "BEARING_WEAR",
    "IMBALANCE",
    "ELECTRICAL"};

static void anomaly_event_cb(const anomaly_detector_result_t *result, void *ctx)
{
    (void)ctx;
    if (!result)
    {
        return;
    }

    const char *label = "UNKNOWN";
    if (result->classification == ANOMALY_CLASS_EXTERNAL_EVENT)
    {
        label = "EXTERNAL_EVENT";
    }
    else if (result->model_class < ANOMALY_MODEL_CLASS_COUNT)
    {
        label = MODEL_CLASS_LABELS[result->model_class];
    }

    if (result->is_anomaly)
    {
        ESP_LOGW(TAG,
                 "Anomaly detected: %s (idx=%u) Pn=%.3f Pb=%.3f Pi=%.3f Pe=%.3f vib_rms=%.4f curr_mean=%.3f",
                 label,
                 (unsigned)result->model_class,
                 result->probability_normal,
                 result->probability_bearing_wear,
                 result->probability_imbalance,
                 result->probability_electrical,
                 result->features[0],
                 result->features[4]);
    }
    else
    {
        ESP_LOGI(TAG,
                 "Inference stable: %s Pn=%.3f Pb=%.3f Pi=%.3f Pe=%.3f vib_rms=%.4f curr_mean=%.3f",
                 label,
                 result->probability_normal,
                 result->probability_bearing_wear,
                 result->probability_imbalance,
                 result->probability_electrical,
                 result->features[0],
                 result->features[4]);
    }
}

void app_status_update_cb(network_status_t status)
{
    switch (status)
    {
    case NETWORK_STATUS_INITIALIZING:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Initializing...");
        break;
    case NETWORK_STATUS_CONNECTING_WIFI:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Connecting Wi-Fi...");
        break;
    case NETWORK_STATUS_CONNECTING_MQTT:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Connecting Broker...");
        break;
    case NETWORK_STATUS_CONNECTED_INTERNET:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_ONLINE);
        data_publisher_start();
        break;
    case NETWORK_STATUS_CONNECTION_FAILED:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_OFFLINE);
        ESP_LOGI(TAG, "LCD UPDATE: Connection Failed.");
        break;
    case NETWORK_STATUS_STARTING_AP_MODE:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Starting AP Mode...");
        break;
    case NETWORK_STATUS_STARTING_LOCAL_BROKER:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: Starting Broker...");
        break;
    case NETWORK_STATUS_AP_MODE_ACTIVE:
        lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_CONNECTING);
        ESP_LOGI(TAG, "LCD UPDATE: AP Active at 192.168.4.1");
        break;
    }
}

void power_off_system(void)
{
    ESP_LOGI(TAG, "Powering off system...");

    set_fan_speed(0);
    set_vibration_speed(0);
    led_manager_stop_rainbow();
    led_manager_clear();
    lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_OFFLINE);

    ui_controller_cleanup();

    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "Entering deep sleep");
    esp_deep_sleep_start();
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_ERROR_CHECK(ret);

    motor_manager_init();

    if (sensor_manager_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensor initialization failed. Halting.");
        return;
    }

#ifdef CONFIG_COLLECTION_MODE
    data_collector_start((collection_mode_t)CONFIG_COLLECTION_MODE);
#else
    // business_logic_start();
    touch_slider_init();

    lcd_manager_start();
    lcd_manager_set_connectivity_state(LCD_CONNECTIVITY_STATE_OFFLINE);

    esp_err_t anomaly_rc = anomaly_detector_init(anomaly_event_cb, NULL, 0.9f);
    if (anomaly_rc != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start anomaly detector (%s)", esp_err_to_name(anomaly_rc));
    }

    network_manager_start(app_status_update_cb);
    led_manager_init();
    led_manager_set_global_brightness(0.25f);
    ESP_ERROR_CHECK(ui_controller_init());
    // fault_simulator_run(FAULT_TYPE_IMBALANCE, 20000);
    // vTaskDelay(pdMS_TO_TICKS(20000));
    // fault_simulator_run(FAULT_TYPE_ELECTRICAL, 20000);
    // fault_simulator_run(FAULT_TYPE_IMBALANCE, 20000);
    // fault_simulator_run(FAULT_TYPE_NORMAL, 20000);

#endif

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
