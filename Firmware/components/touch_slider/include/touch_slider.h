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
 * @brief Gets the last known position of the touch slider while touched.
 * @return uint32_t Current slider position (0-100) or UINT32_MAX if untouched.
 */
uint32_t touch_slider_get_position(void);

/**
 * @brief Reports if the user is currently sliding.
 * @return true when a slide gesture is in progress.
 */
bool touch_slider_is_sliding(void);

/**
 * @brief Checks if a double touch event has been detected.
 * This function is "single-shot"; it resets the flag after being called.
 * Call this in your main loop to poll for the event.
 * @return true if a double touch was detected since the last call, false otherwise.
 */
bool touch_slider_was_double_touched(void);

/**
 * @brief Checks for a single tap after the double-tap timeout expires.
 * @return true once when a single tap is confirmed.
 */
bool touch_slider_was_single_touched(void);

/**
 * @brief Returns the first position recorded when the slider was touched.
 * @return uint32_t The initial touch position (0-100).
 */
uint32_t touch_slider_get_first_touch_position(void);


#endif // TOUCH_SLIDER_H
