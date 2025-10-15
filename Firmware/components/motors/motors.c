#include "motors.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES LEDC_TIMER_8_BIT

#define LEDC_TIMER_FAN LEDC_TIMER_2
#define LEDC_TIMER_VIBRATION LEDC_TIMER_3
#define LEDC_CHANNEL_FAN LEDC_CHANNEL_0
#define LEDC_CHANNEL_VIBRATION LEDC_CHANNEL_1
#define FAN_MIN_PERCENT 70 // fan won’t spin reliably below ~70%

#define LEDC_MAX_DUTY ((1 << 8) - 1)

static const char *TAG = "MOTORS";

void motors_init()
{
    ledc_timer_config_t ledc_timer_fan = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_FAN,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = CONFIG_FAN_LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_fan));

    ledc_timer_config_t ledc_timer_vibration = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER_VIBRATION,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = CONFIG_VIBRATION_LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_vibration));

    ledc_channel_config_t ledc_channel_fan = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_FAN,
        .timer_sel = LEDC_TIMER_FAN,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = CONFIG_FAN_PIN,
        .duty = 0,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_fan));

    ledc_channel_config_t ledc_channel_vibration = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL_VIBRATION,
        .timer_sel = LEDC_TIMER_VIBRATION,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = CONFIG_VIBRATION_PIN,
        .duty = 0,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_vibration));

    ESP_LOGI(TAG, "Motor component initialized");
    ESP_LOGI(TAG, "Fan pin: %d, Freq: %d Hz", CONFIG_FAN_PIN, CONFIG_FAN_LEDC_FREQUENCY);
    ESP_LOGI(TAG, "Vibration pin: %d, Freq: %d Hz", CONFIG_VIBRATION_PIN, CONFIG_VIBRATION_LEDC_FREQUENCY);
}

void set_vibration_speed(uint8_t percentage)
{
    if (percentage > 100)
    {
        percentage = 100;
    }
    uint32_t duty = (LEDC_MAX_DUTY * percentage) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_VIBRATION, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_VIBRATION));
}

// Map a [0–100] PID output into [FAN_MIN_PERCENT–100]
static inline uint8_t map_to_fan_range(uint8_t pid_percent)
{
    if (pid_percent == 0)
    {
        return 0; // allow true OFF when PID says 0
    }

    // Scale proportionally into [FAN_MIN_PERCENT, 100]
    float scaled = FAN_MIN_PERCENT +
                   (pid_percent / 100.0f) * (100 - FAN_MIN_PERCENT);

    return (uint8_t)(scaled + 0.5f); // round
}
void set_fan_speed(uint8_t pid_percent)
{
    uint8_t percentage = map_to_fan_range(pid_percent);

    // Kick-start logic
    static bool fan_running = false;
    if (!fan_running && percentage > 0)
    {
        uint32_t full_duty = LEDC_MAX_DUTY; // 100% duty
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, full_duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN));
        vTaskDelay(pdMS_TO_TICKS(100)); // kick duration
        fan_running = true;
    }
    else if (percentage == 0)
    {
        fan_running = false;
    }

    uint32_t duty = (LEDC_MAX_DUTY * percentage) / 100;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_FAN, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_FAN));
}
