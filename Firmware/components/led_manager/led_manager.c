#include "led_manager.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_STRIP_LED_COUNT 1           // Only one WS2812B 4020
#define LED_STRIP_GPIO_PIN 18
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

static const char *TAG = "led_manager";

static led_strip_handle_t s_led_strip = NULL;
static TaskHandle_t s_rainbow_task = NULL;
static bool s_rainbow_running = false;

// --------- Rainbow Task ---------
static void rainbow_task(void *arg)
{
    uint16_t hue = 0;
    while (s_rainbow_running)
    {
        // Simple rainbow hue wheel
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

        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, r, g, b));
        ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));

        hue = (hue + 1) % 360;
        vTaskDelay(pdMS_TO_TICKS(30)); // ~33 fps smooth rainbow
    }
    vTaskDelete(NULL);
}

// --------- API Implementation ---------
esp_err_t led_manager_init(void)
{
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
    ESP_LOGI(TAG, "LED Manager initialized (1 LED at GPIO %d)", LED_STRIP_GPIO_PIN);
    return ESP_OK;
}

void led_manager_show_normal(uint8_t r, uint8_t g, uint8_t b)
{
    if (s_rainbow_running)
    {
        led_manager_stop_rainbow();
    }
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void led_manager_start_rainbow(void)
{
    if (s_rainbow_running) return;
    s_rainbow_running = true;
    xTaskCreate(rainbow_task, "rainbow_task", 2048, NULL, 5, &s_rainbow_task);
}

void led_manager_stop_rainbow(void)
{
    if (!s_rainbow_running) return;
    s_rainbow_running = false;
    s_rainbow_task = NULL;
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}
