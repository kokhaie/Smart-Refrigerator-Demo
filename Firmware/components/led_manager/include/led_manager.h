#ifndef LED_MANAGER_H
#define LED_MANAGER_H
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    LED_MODE_IDLE = 0,
    LED_MODE_NORMAL,
    LED_MODE_ECO,
    LED_MODE_FREEZER,
    LED_MODE_ERROR,
    LED_MODE_SLIDER_FEEDBACK,
    LED_MODE_RAINBOW, // 🌈 rainbow while not connected
} led_mode_t;

// Initialize LED manager (single WS2812B on GPIO18)
esp_err_t led_manager_init(void);

// Show normal mode (steady white or eco indicator)
void led_manager_show_normal(uint8_t r, uint8_t g, uint8_t b);

// Show rainbow animation (startup/demo)
void led_manager_start_rainbow(void);

// Stop rainbow animation and go back to normal mode
void led_manager_stop_rainbow(void);
#endif
