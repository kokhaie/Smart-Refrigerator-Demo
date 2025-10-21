// led_manager.h (Enhanced version)
#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// LED strip configuration - single LED with diffuser for wide glow effect
#define LED_STRIP_LED_COUNT 1  // Single WS2812B with diffuser
#define LED_STRIP_GPIO_PIN 18

// Initialize LED manager
void led_manager_init(void);

// Basic single LED control (for single LED strips)
void led_manager_show_normal(uint8_t r, uint8_t g, uint8_t b);

// Rainbow animation (startup)
void led_manager_start_rainbow(void);
void led_manager_stop_rainbow(void);

// Multi-LED strip functions
void led_manager_show_slider_bar(uint32_t position, float deviation_ratio, uint8_t r, uint8_t g, uint8_t b);
void led_manager_show_breathing(uint8_t max_intensity);
void led_manager_show_breathing_color(uint8_t r, uint8_t g, uint8_t b,
                                      uint8_t min_intensity, uint8_t max_intensity,
                                      uint32_t period_ms);
void led_manager_show_pulse(uint8_t r, uint8_t g, uint8_t b, uint32_t duration_ms);
void led_manager_clear(void);

// Advanced effects
void led_manager_show_temperature_gradient(float current_temp, float setpoint, float min_temp, float max_temp);
void led_manager_show_error_flash(void);
void led_manager_show_success_flash(void);

/**
 * @brief Adjust the global brightness applied to every LED effect.
 *
 * @param normalized A value between 0.0 (off) and 1.0 (full brightness).
 */
void led_manager_set_global_brightness(float normalized);

#endif // LED_MANAGER_H
