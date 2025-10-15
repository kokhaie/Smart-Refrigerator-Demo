#include "touch_slider.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "touch_element/touch_slider.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdlib.h>
#include <limits.h>

static const char *TAG = "TouchSlider";

// --- Tunable Parameters --- //
#define TAP_SLIDE_TOLERANCE 10
#define DOUBLE_TOUCH_TIME_MS 300
#define DOUBLE_TOUCH_POS_THRESHOLD 15

// --- Hardware & Calibration --- //
#define TOUCH_SLIDER_CHANNEL_NUM 5

static const touch_pad_t channel_array[TOUCH_SLIDER_CHANNEL_NUM] = {
    CONFIG_TOUCH_SLIDER_PAD1_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD2_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD3_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD4_CHANNEL,
    CONFIG_TOUCH_SLIDER_PAD5_CHANNEL,
};

static const float channel_sens_array[TOUCH_SLIDER_CHANNEL_NUM] = {
    0.01F,
    0.01F,
    0.01F,
    0.01F,
    0.01F,
};

// --- State Machine and Variables --- //
typedef enum
{
    STATE_IDLE,
    STATE_TOUCHED,
    STATE_SLIDING,
    STATE_WAITING_FOR_SECOND_TOUCH
} touch_state_t;

static volatile touch_state_t g_slider_state = STATE_IDLE;
static volatile uint64_t g_last_release_time = 0;
static volatile uint32_t g_first_touch_pos = 0;
static volatile int g_slide_event_count = 0;
static volatile uint32_t g_slider_position = 0;
static volatile bool g_double_touch_detected = false;
static volatile bool g_single_touch_detected = false;
static volatile bool g_is_currently_touched = false;  // NEW: Track if finger is on slider

// --- Main Event Callback --- //
static void slider_event_callback(touch_slider_handle_t out_handle, touch_slider_message_t *msg, void *arg)
{
    g_slider_position = msg->position;

    switch (msg->event)
    {
    case TOUCH_SLIDER_EVT_ON_PRESS:
    {
        g_is_currently_touched = true;  // NEW: Mark as touched
        
        if (g_slider_state == STATE_WAITING_FOR_SECOND_TOUCH)
        {
            uint64_t now = esp_timer_get_time();
            uint64_t time_since_release = (now - g_last_release_time) / 1000;
            int pos_diff = abs((int)msg->position - (int)g_first_touch_pos);

            if (time_since_release < DOUBLE_TOUCH_TIME_MS &&
                pos_diff < DOUBLE_TOUCH_POS_THRESHOLD &&
                g_slide_event_count <= TAP_SLIDE_TOLERANCE)
            {
                g_double_touch_detected = true;
                ESP_LOGI(TAG, ">>> Double Touch Detected at position %lu! <<<", msg->position);
                g_slider_state = STATE_TOUCHED;
                g_slide_event_count = 0;
            }
            else
            {
                g_slider_state = STATE_TOUCHED;
                g_first_touch_pos = msg->position;
                g_slide_event_count = 0;
            }
        }
        else
        {
            g_slider_state = STATE_TOUCHED;
            g_first_touch_pos = msg->position;
            g_slide_event_count = 0;
            ESP_LOGI(TAG, "Touch started at position %lu", msg->position);
        }
        break;
    }

    case TOUCH_SLIDER_EVT_ON_RELEASE:
    {
        g_is_currently_touched = false;  // NEW: Mark as released
        
        if (g_slide_event_count <= TAP_SLIDE_TOLERANCE)
        {
            // This was a tap, not a slide
            g_slider_state = STATE_WAITING_FOR_SECOND_TOUCH;
            g_last_release_time = esp_timer_get_time();
            ESP_LOGI(TAG, "Tap detected, waiting for second touch");
        }
        else
        {
            // This was a slide gesture
            ESP_LOGI(TAG, "Slide gesture completed (%d events)", g_slide_event_count);
            g_slider_state = STATE_IDLE;
        }
        g_slide_event_count = 0;
        break;
    }

    case TOUCH_SLIDER_EVT_ON_CALCULATION:
    {
        g_slide_event_count++;
        
        // NEW: Transition to SLIDING state after tolerance exceeded
        if (g_slide_event_count > TAP_SLIDE_TOLERANCE && g_slider_state == STATE_TOUCHED)
        {
            g_slider_state = STATE_SLIDING;
            ESP_LOGI(TAG, "Started sliding from position %lu", g_first_touch_pos);
        }
        
        // Log position updates during sliding
        if (g_slider_state == STATE_SLIDING)
        {
            ESP_LOGI(TAG, "Sliding: position = %lu", msg->position);
        }
        break;
    }

    default:
        break;
    }
}

// --- Public API Functions --- //

esp_err_t touch_slider_init(void)
{
    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_element_install(&global_config));
    touch_slider_global_config_t slider_global_config = TOUCH_SLIDER_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_slider_install(&slider_global_config));

    touch_slider_config_t slider_config = {
        .channel_array = channel_array,
        .sensitivity_array = channel_sens_array,
        .channel_num = TOUCH_SLIDER_CHANNEL_NUM,
        .position_range = 101};

    touch_slider_handle_t slider_handle;
    ESP_ERROR_CHECK(touch_slider_create(&slider_config, &slider_handle));

    ESP_ERROR_CHECK(touch_slider_subscribe_event(slider_handle,
                                                 TOUCH_ELEM_EVENT_ON_PRESS |
                                                     TOUCH_ELEM_EVENT_ON_RELEASE |
                                                     TOUCH_ELEM_EVENT_ON_CALCULATION,
                                                 NULL));

    ESP_ERROR_CHECK(touch_slider_set_dispatch_method(slider_handle, TOUCH_ELEM_DISP_CALLBACK));
    ESP_ERROR_CHECK(touch_slider_set_callback(slider_handle, slider_event_callback));

    ESP_ERROR_CHECK(touch_element_start());
    ESP_LOGI(TAG, "Touch slider initialized successfully.");
    return ESP_OK;
}

uint32_t touch_slider_get_position(void)
{
    // NEW: Return position whenever finger is on the slider
    if (g_is_currently_touched)
    {
        return g_slider_position;
    }
    return UINT32_MAX;
}

bool touch_slider_is_sliding(void)
{
    // NEW: Check if currently in a slide gesture
    return (g_slider_state == STATE_SLIDING);
}

bool touch_slider_was_double_touched(void)
{
    if (g_double_touch_detected)
    {
        g_double_touch_detected = false;
        return true;
    }
    return false;
}

bool touch_slider_was_single_touched(void)
{
    if (g_slider_state == STATE_WAITING_FOR_SECOND_TOUCH)
    {
        uint64_t time_since_release_us = esp_timer_get_time() - g_last_release_time;
        if (time_since_release_us > (DOUBLE_TOUCH_TIME_MS * 1000))
        {
            ESP_LOGI(TAG, "Single Tap Detected (timeout) at position %lu", g_first_touch_pos);
            g_slider_state = STATE_IDLE;
            g_single_touch_detected = true;
        }
    }

    if (g_single_touch_detected)
    {
        g_single_touch_detected = false;
        return true;
    }
    return false;
}

uint32_t touch_slider_get_first_touch_position(void)
{
    // NEW: Get the initial touch position (useful for tap detection)
    return g_first_touch_pos;
}