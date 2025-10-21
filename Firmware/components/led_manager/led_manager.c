#include "led_manager.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"

#include <math.h>

#define LED_STRIP_LED_COUNT 1           // Single WS2812B with diffuser for wide glow effect
#define LED_STRIP_GPIO_PIN 18
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)
#define TWO_PI_F 6.283185307f

static const char *TAG = "led_manager";

typedef enum
{
    LED_EFFECT_NONE = 0,
    LED_EFFECT_RAINBOW,
    LED_EFFECT_STATIC,
    LED_EFFECT_BREATHING,
    LED_EFFECT_SLIDER,
    LED_EFFECT_PULSE,
    LED_EFFECT_TEMPERATURE,
    LED_EFFECT_ERROR
} led_effect_t;

static led_strip_handle_t s_led_strip = NULL;
static TaskHandle_t s_rainbow_task = NULL;
static bool s_rainbow_running = false;
static led_effect_t s_active_effect = LED_EFFECT_NONE;
static SemaphoreHandle_t s_led_lock = NULL;
static float s_brightness_scale = 0.35f;

static void rainbow_task(void *arg);
static inline void led_lock(void);
static inline void led_unlock(void);
static inline void led_apply_rgb(uint8_t r, uint8_t g, uint8_t b);
static void led_stop_rainbow_locked(bool clear_strip);

static inline void led_lock(void)
{
    if (s_led_lock != NULL)
    {
        xSemaphoreTakeRecursive(s_led_lock, portMAX_DELAY);
    }
}

static inline void led_unlock(void)
{
    if (s_led_lock != NULL)
    {
        xSemaphoreGiveRecursive(s_led_lock);
    }
}

static inline uint8_t led_apply_scale(uint8_t component)
{
    float scaled = (float)component * s_brightness_scale;
    if (scaled < 0.0f)
    {
        scaled = 0.0f;
    }
    else if (scaled > 255.0f)
    {
        scaled = 255.0f;
    }
    return (uint8_t)lroundf(scaled);
}

static inline void led_apply_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t sr = led_apply_scale(r);
    uint8_t sg = led_apply_scale(g);
    uint8_t sb = led_apply_scale(b);
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, sr, sg, sb));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

static void led_stop_rainbow_locked(bool clear_strip)
{
    if (!s_rainbow_running)
    {
        return;
    }

    s_rainbow_running = false;
    s_active_effect = LED_EFFECT_NONE;

    if (clear_strip)
    {
        ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
        ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
    }
}

static void rainbow_task(void *arg)
{
    (void)arg;
    uint16_t hue = 0;

    while (s_rainbow_running)
    {
        uint8_t r, g, b;
        uint8_t region = hue / 60;
        uint8_t remainder = (hue % 60) * 255 / 60;

        switch (region)
        {
        case 0: r = 255; g = remainder; b = 0; break;
        case 1: r = 255 - remainder; g = 255; b = 0; break;
        case 2: r = 0; g = 255; b = remainder; break;
        case 3: r = 0; g = 255 - remainder; b = 255; break;
        case 4: r = remainder; g = 0; b = 255; break;
        default: r = 255; g = 0; b = 255 - remainder; break;
        }

        led_lock();
        if (!s_rainbow_running)
        {
            led_unlock();
            break;
        }
        led_apply_rgb(r, g, b);
        led_unlock();

        hue = (hue + 1) % 360;
        vTaskDelay(pdMS_TO_TICKS(30)); // ~33 fps smooth rainbow
    }

    led_lock();
    s_active_effect = LED_EFFECT_NONE;
    led_unlock();

    vTaskDelete(NULL);
}

void led_manager_init(void)
{
    if (s_led_lock == NULL)
    {
        s_led_lock = xSemaphoreCreateRecursiveMutex();
        if (s_led_lock == NULL)
        {
            ESP_LOGE(TAG, "Failed to create LED manager mutex");
            return;
        }
    }

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO_PIN,
        .max_leds = LED_STRIP_LED_COUNT,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip));
    s_active_effect = LED_EFFECT_NONE;
    ESP_LOGI(TAG, "LED Manager initialized (1 LED at GPIO %d)", LED_STRIP_GPIO_PIN);
    ESP_LOGI(TAG, "Default LED brightness scale set to %.0f%%", s_brightness_scale * 100.0f);
}

void led_manager_show_normal(uint8_t r, uint8_t g, uint8_t b)
{
    led_lock();
    led_stop_rainbow_locked(false);
    s_active_effect = LED_EFFECT_STATIC;
    led_apply_rgb(r, g, b);
    led_unlock();
}

void led_manager_start_rainbow(void)
{
    led_lock();
    if (s_rainbow_running)
    {
        led_unlock();
        return;
    }

    s_rainbow_running = true;
    s_active_effect = LED_EFFECT_RAINBOW;
    BaseType_t created = xTaskCreate(rainbow_task, "rainbow_task", 2048, NULL, 5, &s_rainbow_task);
    if (created != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to start rainbow task");
        s_rainbow_running = false;
        s_active_effect = LED_EFFECT_NONE;
    }
    led_unlock();
}

void led_manager_stop_rainbow(void)
{
    led_lock();
    led_stop_rainbow_locked(true);
    led_unlock();
}

void led_manager_show_slider_bar(uint32_t position, float deviation_ratio, uint8_t r, uint8_t g, uint8_t b)
{
    if (position > 100)
    {
        position = 100;
    }

    float clamped = fminf(fmaxf(deviation_ratio, 0.0f), 1.0f);
    float position_ratio = (float)position / 100.0f;
    uint8_t intensity = (uint8_t)(60.0f + position_ratio * 40.0f + clamped * 155.0f);

    uint8_t final_r = (r * intensity) / 255;
    uint8_t final_g = (g * intensity) / 255;
    uint8_t final_b = (b * intensity) / 255;

    led_lock();
    led_stop_rainbow_locked(false);
    s_active_effect = LED_EFFECT_SLIDER;
    led_apply_rgb(final_r, final_g, final_b);
    led_unlock();
}

void led_manager_show_breathing(uint8_t max_intensity)
{
    uint8_t capped_max = (max_intensity < 40U) ? 40U : max_intensity;
    led_manager_show_breathing_color(255, 255, 255, 24, capped_max, 7000);
}

void led_manager_show_breathing_color(uint8_t r, uint8_t g, uint8_t b,
                                      uint8_t min_intensity, uint8_t max_intensity,
                                      uint32_t period_ms)
{
    if (max_intensity < min_intensity)
    {
        uint8_t tmp = max_intensity;
        max_intensity = min_intensity;
        min_intensity = tmp;
    }

    if (period_ms < 800U)
    {
        period_ms = 800U;
    }

    uint32_t now_ms = esp_timer_get_time() / 1000U;
    uint32_t phase_ms = (period_ms == 0U) ? 0U : (now_ms % period_ms);
    float phase = (period_ms == 0U) ? 0.0f : ((float)phase_ms / (float)period_ms);

    float wave = (1.0f - cosf(phase * TWO_PI_F)) * 0.5f; // Eased 0-1 curve
    float intensity = (float)min_intensity + wave * (float)(max_intensity - min_intensity);
    float brightness = fminf(intensity / 255.0f, 1.0f);

    uint8_t final_r = (uint8_t)fminf((float)r * brightness, 255.0f);
    uint8_t final_g = (uint8_t)fminf((float)g * brightness, 255.0f);
    uint8_t final_b = (uint8_t)fminf((float)b * brightness, 255.0f);

    led_lock();
    led_stop_rainbow_locked(false);
    s_active_effect = LED_EFFECT_BREATHING;
    led_apply_rgb(final_r, final_g, final_b);
    led_unlock();
}

void led_manager_show_pulse(uint8_t r, uint8_t g, uint8_t b, uint32_t duration_ms)
{
    if (duration_ms == 0)
    {
        duration_ms = 1;
    }

    uint32_t start_time = esp_timer_get_time() / 1000;
    uint32_t elapsed_time = 0;

    led_lock();
    led_stop_rainbow_locked(false);
    s_active_effect = LED_EFFECT_PULSE;
    led_unlock();

    while (elapsed_time < duration_ms)
    {
        elapsed_time = esp_timer_get_time() / 1000 - start_time;
        float progress = (float)elapsed_time / duration_ms;
        float intensity = (1.0f + sinf(progress * TWO_PI_F)) / 2.0f;

        uint8_t pulse_r = (uint8_t)(r * intensity);
        uint8_t pulse_g = (uint8_t)(g * intensity);
        uint8_t pulse_b = (uint8_t)(b * intensity);

        led_lock();
        led_apply_rgb(pulse_r, pulse_g, pulse_b);
        led_unlock();

        vTaskDelay(pdMS_TO_TICKS(20)); // 50 FPS
    }

    led_lock();
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
    s_active_effect = LED_EFFECT_NONE;
    led_unlock();
}

void led_manager_show_temperature_gradient(float current_temp, float setpoint, float min_temp, float max_temp)
{
    uint8_t r, g, b;

    float temp_diff = current_temp - setpoint;
    float tolerance = 1.0f; // 1°C tolerance

    if (fabsf(temp_diff) <= tolerance)
    {
        r = 0; g = 255; b = 0;
    }
    else if (temp_diff > tolerance)
    {
        float ratio = fminf(fabsf(temp_diff) / (max_temp - setpoint), 1.0f);
        r = 255;
        g = (uint8_t)(255.0f * (1.0f - ratio));
        b = 0;
    }
    else
    {
        float ratio = fminf(fabsf(temp_diff) / (setpoint - min_temp), 1.0f);
        r = 0;
        g = (uint8_t)(255.0f * (1.0f - ratio));
        b = 255;
    }

    float intensity_factor = 1.0f;
    if (fabsf(temp_diff) > tolerance)
    {
        intensity_factor = 0.6f + 0.4f * (1.0f - fminf(fabsf(temp_diff) / 5.0f, 1.0f));
    }

    uint8_t final_r = (uint8_t)(r * intensity_factor);
    uint8_t final_g = (uint8_t)(g * intensity_factor);
    uint8_t final_b = (uint8_t)(b * intensity_factor);

    led_lock();
    led_stop_rainbow_locked(false);
    s_active_effect = LED_EFFECT_TEMPERATURE;
    led_apply_rgb(final_r, final_g, final_b);
    led_unlock();
}

void led_manager_show_error_flash(void)
{
    led_manager_show_pulse(255, 0, 0, 400);
}

void led_manager_show_success_flash(void)
{
    led_manager_show_pulse(0, 255, 0, 300);
}

void led_manager_clear(void)
{
    led_lock();
    led_stop_rainbow_locked(false);
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
    s_active_effect = LED_EFFECT_NONE;
    led_unlock();
}

void led_manager_set_global_brightness(float normalized)
{
    if (isnan(normalized) || isinf(normalized))
    {
        ESP_LOGW(TAG, "Ignoring invalid brightness value: %f", (double)normalized);
        return;
    }

    float clamped = normalized;
    if (clamped < 0.0f)
    {
        clamped = 0.0f;
    }
    else if (clamped > 1.0f)
    {
        clamped = 1.0f;
    }

    led_lock();
    s_brightness_scale = clamped;
    led_unlock();

    ESP_LOGI(TAG, "LED brightness scale set to %.0f%%", clamped * 100.0f);
}
