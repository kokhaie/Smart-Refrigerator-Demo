#include "motors.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "sdkconfig.h" 
// Define the LEDC timer and channel configurations
#define LEDC_MODE              LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES          LEDC_TIMER_8_BIT 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Timer and Channel assignments
#define LEDC_TIMER_FAN         LEDC_TIMER_2
#define LEDC_TIMER_VIBRATION   LEDC_TIMER_3
#define LEDC_CHANNEL_FAN       LEDC_CHANNEL_0
#define LEDC_CHANNEL_VIBRATION LEDC_CHANNEL_1

// Calculate the maximum duty cycle value based on the resolution
#define LEDC_MAX_DUTY ((1 << 8) - 1)
static const char *TAG = "MOTORS";

void motors_init() {
    // 1. Configure timer for the Fan
    ledc_timer_config_t ledc_timer_fan = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER_FAN,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = CONFIG_FAN_LEDC_FREQUENCY, // Use Fan frequency from Kconfig
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_fan));

    // 2. Configure timer for the Vibration motor
    ledc_timer_config_t ledc_timer_vibration = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER_VIBRATION,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = CONFIG_VIBRATION_LEDC_FREQUENCY, // Use Vibration frequency from Kconfig
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_vibration));

    // 3. Configure channel for the Fan, linking it to the fan's timer
    ledc_channel_config_t ledc_channel_fan = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_FAN,
        .timer_sel      = LEDC_TIMER_FAN, // Link to fan timer
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = CONFIG_FAN_PIN, // Use value from Kconfig
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_fan));

    // 4. Configure channel for the Vibration motor, linking it to the vibration's timer
    ledc_channel_config_t ledc_channel_vibration = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_VIBRATION,
        .timer_sel      = LEDC_TIMER_VIBRATION, // Link to vibration timer
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = CONFIG_VIBRATION_PIN, // Use value from Kconfig
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_vibration));
    
    ESP_LOGI(TAG, "Motor component initialized");
    ESP_LOGI(TAG, "Fan pin: %d, Freq: %d Hz", CONFIG_FAN_PIN, CONFIG_FAN_LEDC_FREQUENCY);
    ESP_LOGI(TAG, "Vibration pin: %d, Freq: %d Hz", CONFIG_VIBRATION_PIN, CONFIG_VIBRATION_LEDC_FREQUENCY);
}

void set_vibration_speed(uint8_t percentage) {
    if (percentage > 100) {
        percentage = 100;
    }
    uint32_t duty = (LEDC_MAX_DUTY * percentage) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_VIBRATION, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_VIBRATION));
}
void set_fan_speed(uint8_t percentage) {
    if (percentage > 100) {
        percentage = 100;
    }

    // If the fan is being started at a low speed, give it a kick
    if (percentage > 0 && percentage < 40) { // Kick-start threshold is 40%
        // Apply 100% power for 20 milliseconds
        uint32_t full_duty = (LEDC_MAX_DUTY * 100) / 100;
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, full_duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN));
        vTaskDelay(pdMS_TO_TICKS(20)); // Short delay for the kick
    }

    // Now set the actual desired speed
    uint32_t duty = (LEDC_MAX_DUTY * percentage) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN));
}