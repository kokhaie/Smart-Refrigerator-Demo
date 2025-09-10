// touch_slider.h

#ifndef TOUCH_SLIDER_H
#define TOUCH_SLIDER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h> // Include for boolean type

/**
 * @brief Initializes the touch slider.
 * Call this function once at application startup.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t touch_slider_init(void);

/**
 * @brief Gets the last known position of the touch slider.
 * This is a non-blocking function.
 * @return uint32_t The current slider position (0-100).
 */
uint32_t touch_slider_get_position(void);

/**
 * @brief Checks if a double touch event has been detected.
 * This function is "single-shot"; it resets the flag after being called.
 * Call this in your main loop to poll for the event.
 * @return true if a double touch was detected since the last call, false otherwise.
 */
bool touch_slider_was_double_touched(void);


#endif // TOUCH_SLIDER_H