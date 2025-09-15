#include "touch_slider.h"
#include "freertos/FreeRTOS.h"
#include "touch_element/touch_slider.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdlib.h>

static const char *TAG = "TouchSlider";
#define TAP_SLIDE_TOLERANCE 10
// --- Double Touch Configuration --- //
#define DOUBLE_TOUCH_TIME_MS 300      // Max time in ms between two taps
#define DOUBLE_TOUCH_POS_THRESHOLD 15 // Max position change allowed for a double tap

// --- Hardware & Calibration --- //
#define TOUCH_SLIDER_CHANNEL_NUM 5

// Assign the physical touch pads (GPIOs) that make up the slider.
static const touch_pad_t channel_array[TOUCH_SLIDER_CHANNEL_NUM] = {
    CONFIG_TOUCH_SLIDER_PAD1_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD2_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD3_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD4_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD5_CHANNEL,
};

// Sensitivity calibration values for each channel.
static const float channel_sens_array[TOUCH_SLIDER_CHANNEL_NUM] = {
    0.252F,
    0.246F,
    0.277F,
    0.250F,
    0.257F,
};

// --- State Machine and Variables --- //
typedef enum
{
    STATE_IDLE,                    // Waiting for any touch
    STATE_WAITING_FOR_SECOND_TOUCH // A tap was just released, waiting for the next one
} touch_state_t;

static volatile touch_state_t g_slider_state = STATE_IDLE;
static volatile uint64_t g_last_release_time = 0;
static volatile uint32_t g_first_touch_pos = 0;
static volatile int g_slide_event_count = 0;

// Publicly readable state variables
static volatile uint32_t g_slider_position = 0;
static volatile bool g_double_touch_detected = false;

// --- Main Event Callback --- //
// --- Main Event Callback (NEW DEBUG VERSION) --- //
static void slider_event_callback(touch_slider_handle_t out_handle, touch_slider_message_t *msg, void *arg)
{
    // Always update the current position for the main app to read
    g_slider_position = msg->position;

    switch (msg->event)
    {
    case TOUCH_SLIDER_EVT_ON_PRESS:
    {
        uint64_t now = esp_timer_get_time(); // Get time in microseconds

        if (g_slider_state == STATE_WAITING_FOR_SECOND_TOUCH)
        {
            uint64_t time_since_release = (now - g_last_release_time) / 1000; // in ms
            int pos_diff = abs((int)msg->position - (int)g_first_touch_pos);

            // --- START OF DEBUG LOGS ---
            ESP_LOGW(TAG, "Double-tap check: time=%llums, pos_diff=%d, slide_count=%d",
                     time_since_release, pos_diff, g_slide_event_count);
            // --- END OF DEBUG LOGS ---

            // Check all conditions for a valid double touch
            if (time_since_release < DOUBLE_TOUCH_TIME_MS &&
                pos_diff < DOUBLE_TOUCH_POS_THRESHOLD &&
                g_slide_event_count <=  TAP_SLIDE_TOLERANCE)
            {
                // SUCCESS: Double touch detected!
                g_double_touch_detected = true;
                ESP_LOGI(TAG, ">>> Double Touch Detected! <<<");
                g_slider_state = STATE_IDLE; // Reset state machine
            }
            else
            {
                // --- START OF DEBUG LOGS ---
                // FAILED: Conditions not met, print the reason
                if (time_since_release >= DOUBLE_TOUCH_TIME_MS)
                {
                    ESP_LOGE(TAG, "Double-tap FAILED because time was too long.");
                }
                if (pos_diff >= DOUBLE_TOUCH_POS_THRESHOLD)
                {
                    ESP_LOGE(TAG, "Double-tap FAILED because position changed too much.");
                }
                if (g_slide_event_count != 0)
                {
                    ESP_LOGE(TAG, "Double-tap FAILED because a slide occurred during the first tap.");
                }
                // --- END OF DEBUG LOGS ---
                g_slider_state = STATE_IDLE;
            }
        }
        // This press is the first in a potential new sequence
        g_first_touch_pos = msg->position;
        g_slide_event_count = 0; // Reset slide counter on any new press
        break;
    }

    case TOUCH_SLIDER_EVT_ON_RELEASE:
    {
        // A touch was released. This could be the first tap of a double tap.
        ESP_LOGW(TAG, "Tap released. Waiting for second touch... (slide_count was %d)", g_slide_event_count);
        g_slider_state = STATE_WAITING_FOR_SECOND_TOUCH;
        g_last_release_time = esp_timer_get_time();
        break;
    }

    case TOUCH_SLIDER_EVT_ON_CALCULATION:
    {
        // A slide event occurred.
        g_slide_event_count++;
        ESP_LOGD(TAG, "Slide event detected (count: %d)", g_slide_event_count); // Use Debug level for less spam
        // If the user starts sliding, it invalidates the double-touch sequence.
        if (g_slider_state == STATE_WAITING_FOR_SECOND_TOUCH)
        {
            g_slider_state = STATE_IDLE;
        }
        break;
    }

    default:
        break;
    }
}
esp_err_t touch_slider_init(void)
{
    // 1. Install general touch element and touch slider libraries.
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_element_install(&global_config));
    touch_slider_global_config_t slider_global_config = TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_slider_install(&slider_global_config));

    // 2. Configure the slider properties.
    touch_slider_config_t slider_config = {
        .channel_array = channel_array,
        .sensitivity_array = channel_sens_array,
        .channel_num = TOUCH_SLIDER_CHANNEL_NUM,
        .position_range = 101 // Set the output range from 0 to 100
    };

    touch_slider_handle_t slider_handle;
    ESP_ERROR_CHECK(touch_slider_create(&slider_config, &slider_handle));

    // 3. Subscribe to ALL events to feed the state machine
    ESP_ERROR_CHECK(touch_slider_subscribe_event(slider_handle,
                                                 TOUCH_ELEM_EVENT_ON_PRESS |
                                                     TOUCH_ELEM_EVENT_ON_RELEASE |
                                                     TOUCH_ELEM_EVENT_ON_CALCULATION,
                                                 NULL));

    // 4. Set the dispatch method to CALLBACK and register our logic
    ESP_ERROR_CHECK(touch_slider_set_dispatch_method(slider_handle, TOUCH_ELEM_DISP_CALLBACK));
    ESP_ERROR_CHECK(touch_slider_set_callback(slider_handle, slider_event_callback));

    // 5. Start the whole touch element system.
    ESP_ERROR_CHECK(touch_element_start());
    ESP_LOGI(TAG, "Touch slider initialized successfully.");
    return ESP_OK;
}

uint32_t touch_slider_get_position(void)
{
    return g_slider_position;
}

bool touch_slider_was_double_touched(void)
{
    if (g_double_touch_detected)
    {
        g_double_touch_detected = false; // Reset the flag after reading
        return true;
    }
    return false;
}