#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// UI State Machine States
typedef enum {
    UI_STATE_BOOTING,      // Rainbow startup animation
    UI_STATE_IDLE,         // Breathing effect waiting for touch
    UI_STATE_TOUCHED,      // User touched the slider
    UI_STATE_SLIDING,      // User is sliding
    UI_STATE_SET_CONFIRMED,// Double tap - set point confirmed
    UI_STATE_ERROR         // Error state - red flash
} ui_state_t;

// Temperature configuration
#define TEMP_MIN_CELSIUS 0.0f
#define TEMP_MAX_CELSIUS 8.0f
#define TEMP_DEFAULT_CELSIUS 5.0f

// UI Configuration
#define SLIDER_TOUCH_THRESHOLD 10    // Touch events before considering it a slide
#define BREATHING_BASE_INTENSITY 100  // Base brightness for breathing effect
#define SLIDER_EDGE_MARGIN_PCT 12.0f  // Increased margin so extremes stay off the physical edges

// Initialize UI controller
esp_err_t ui_controller_init(void);

// Main UI update function - call periodically
void ui_controller_update(void);

// Touch event handlers (called from touch slider callbacks)
void ui_controller_on_touch_start(uint32_t position);
void ui_controller_on_touch_slide(uint32_t position);
void ui_controller_on_touch_release(uint32_t position);
void ui_controller_on_double_tap(void);
void ui_controller_on_single_tap(void);

// Temperature update from sensors
void ui_controller_on_temperature_update(float current_temp);

// Set target temperature (from PID or other components)
void ui_controller_set_target_temperature(float temp);

// Get current UI state
ui_state_t ui_controller_get_state(void);

// Get current target temperature
float ui_controller_get_target_temperature(void);

// Cleanup
void ui_controller_cleanup(void);

#endif // UI_CONTROLLER_H
