#include "ui_controller.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "led_manager.h"
#include "business_logic.h"
#include "touch_slider.h"

#include <math.h>

static const char *TAG = "ui_controller";

#define UI_TASK_STACK_SIZE 4096
#define UI_TASK_PRIORITY 5
#define UI_POLL_INTERVAL_MS 20
#define UI_BOOT_ANIMATION_MIN_MS 3000
#define UI_INTERACTION_IDLE_MS 5000
#define UI_POST_RELEASE_HOLD_MS 1200
#define UI_SINGLE_TAP_WAKE_HOLD_MS 600
#define UI_DOUBLE_TAP_EXTRA_HOLD_MS 400
#define IDLE_TINT_THRESHOLD 0.4f
#define IDLE_ALERT_THRESHOLD 2.0f
#define SLIDER_EMPHASIS_MIN 0.18f
#define SLIDER_RELEASE_EMPHASIS 0.3f

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

static const rgb_color_t COLOR_NEUTRAL_WHITE = {255, 255, 255};
static const rgb_color_t COLOR_IDLE_BASE = {48, 48, 64};
static const rgb_color_t COLOR_IDLE_COOL = {60, 96, 180};
static const rgb_color_t COLOR_IDLE_WARM = {180, 100, 48};
static const rgb_color_t COLOR_IDLE_ALERT = {200, 110, 16};
static const rgb_color_t COLOR_TOUCH_WAKE = {170, 180, 220};
static const rgb_color_t COLOR_SLIDER_NEUTRAL = {230, 230, 230};
static const rgb_color_t COLOR_SLIDER_COOL = {140, 200, 255};
static const rgb_color_t COLOR_SLIDER_WARM = {255, 190, 120};
static const rgb_color_t COLOR_SUCCESS_GLOW = {90, 200, 150};
static const rgb_color_t COLOR_BOOT_FLASH = {220, 220, 220};

typedef struct
{
    bool active;
    uint64_t timestamp_us;
    uint32_t duration_ms;
} timed_flag_t;

static TaskHandle_t s_ui_task_handle = NULL;
static SemaphoreHandle_t s_state_lock = NULL;

static ui_state_t s_ui_state = UI_STATE_BOOTING;
static float s_target_temperature = TEMP_DEFAULT_CELSIUS;
static float s_current_temperature = TEMP_DEFAULT_CELSIUS;
static float s_last_slider_temp_c = TEMP_DEFAULT_CELSIUS;

static uint32_t s_last_slider_position = 0;
static bool s_is_touch_active = false;
static bool s_boot_animation_running = false;
static bool s_slide_detected = false;
static bool s_ignore_next_single_tap = false;
static timed_flag_t s_release_display_hold = {0};

static uint64_t s_boot_start_time_us = 0;
static uint64_t s_last_interaction_time_us = 0;

static void ui_controller_task(void *param);
static void ui_controller_handle_single_tap(void);
static void ui_controller_handle_double_tap(void);
static void ui_controller_handle_touch_start(uint32_t position);
static void ui_controller_handle_touch_slide(uint32_t position);
static void ui_controller_handle_touch_release(uint32_t position);
static void ui_controller_handle_idle_animation(void);
static void ui_controller_transition_to_idle(void);
static float slider_position_to_temperature(uint32_t position);
static void mark_interaction(void);
static bool is_timer_expired(const timed_flag_t *flag);
static void schedule_flag(timed_flag_t *flag, uint32_t timeout_ms);
static void clear_flag(timed_flag_t *flag);
static rgb_color_t rgb_lerp(rgb_color_t from, rgb_color_t to, float t);
static rgb_color_t ui_controller_compute_slider_color(float slider_temp, float reference_temp);
static void ui_controller_render_slider(uint32_t position, float slider_temp, float reference_temp, float deviation_ratio);

static void lock_state(void)
{
    if (s_state_lock != NULL)
    {
        xSemaphoreTake(s_state_lock, portMAX_DELAY);
    }
}

static void unlock_state(void)
{
    if (s_state_lock != NULL)
    {
        xSemaphoreGive(s_state_lock);
    }
}

esp_err_t ui_controller_init(void)
{
    if (s_ui_task_handle != NULL)
    {
        ESP_LOGW(TAG, "UI controller already initialized");
        return ESP_OK;
    }

    s_state_lock = xSemaphoreCreateMutex();
    if (s_state_lock == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate UI controller mutex");
        return ESP_ERR_NO_MEM;
    }

    lock_state();
    s_ui_state = UI_STATE_BOOTING;
    s_target_temperature = TEMP_DEFAULT_CELSIUS;
    s_current_temperature = TEMP_DEFAULT_CELSIUS;
    s_last_slider_position = 0;
    s_is_touch_active = false;
    s_last_slider_temp_c = TEMP_DEFAULT_CELSIUS;
    s_release_display_hold.active = false;
    s_release_display_hold.duration_ms = 0;
    s_boot_animation_running = false;
    s_slide_detected = false;
    s_ignore_next_single_tap = false;
    s_boot_start_time_us = esp_timer_get_time();
    s_last_interaction_time_us = s_boot_start_time_us;
    unlock_state();

    led_manager_start_rainbow();
    s_boot_animation_running = true;

    BaseType_t task_created = xTaskCreate(
        ui_controller_task,
        "ui_controller_task",
        UI_TASK_STACK_SIZE,
        NULL,
        UI_TASK_PRIORITY,
        &s_ui_task_handle);

    if (task_created != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create UI controller task");
        s_ui_task_handle = NULL;
        vSemaphoreDelete(s_state_lock);
        s_state_lock = NULL;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "UI controller initialized");
    return ESP_OK;
}

void ui_controller_cleanup(void)
{
    if (s_ui_task_handle != NULL)
    {
        vTaskDelete(s_ui_task_handle);
        s_ui_task_handle = NULL;
    }

    if (s_state_lock != NULL)
    {
        vSemaphoreDelete(s_state_lock);
        s_state_lock = NULL;
    }

    led_manager_clear();
    s_boot_animation_running = false;
    ESP_LOGI(TAG, "UI controller cleaned up");
}

ui_state_t ui_controller_get_state(void)
{
    ui_state_t state;
    lock_state();
    state = s_ui_state;
    unlock_state();
    return state;
}

float ui_controller_get_target_temperature(void)
{
    float temperature;
    lock_state();
    temperature = s_target_temperature;
    unlock_state();
    return temperature;
}

void ui_controller_set_target_temperature(float temp)
{
    lock_state();
    if (temp < TEMP_MIN_CELSIUS)
    {
        temp = TEMP_MIN_CELSIUS;
    }
    else if (temp > TEMP_MAX_CELSIUS)
    {
        temp = TEMP_MAX_CELSIUS;
    }

    s_target_temperature = temp;
    unlock_state();

    ESP_LOGI(TAG, "Target temperature externally set to %.2f°C", temp);
}

void ui_controller_on_temperature_update(float current_temp)
{
    lock_state();
    s_current_temperature = current_temp;
    unlock_state();
}

void ui_controller_on_touch_start(uint32_t position)
{
    ui_controller_handle_touch_start(position);
}

void ui_controller_on_touch_slide(uint32_t position)
{
    ui_controller_handle_touch_slide(position);
}

void ui_controller_on_touch_release(uint32_t position)
{
    ui_controller_handle_touch_release(position);
}

void ui_controller_on_double_tap(void)
{
    ui_controller_handle_double_tap();
}

void ui_controller_on_single_tap(void)
{
    ui_controller_handle_single_tap();
}

void ui_controller_update(void)
{
    if (s_ui_state == UI_STATE_BOOTING)
    {
        uint64_t elapsed_ms = (esp_timer_get_time() - s_boot_start_time_us) / 1000;
        if (elapsed_ms >= UI_BOOT_ANIMATION_MIN_MS)
        {
            if (s_boot_animation_running)
            {
                led_manager_stop_rainbow();
                s_boot_animation_running = false;
                led_manager_show_pulse(COLOR_BOOT_FLASH.r, COLOR_BOOT_FLASH.g, COLOR_BOOT_FLASH.b, 160);
            }
            ui_controller_transition_to_idle();
        }
        else
        {
            return;
        }
    }

    // Gesture events
    if (touch_slider_was_double_touched())
    {
        ui_controller_handle_double_tap();
    }

    if (touch_slider_was_single_touched())
    {
        if (s_ignore_next_single_tap)
        {
            s_ignore_next_single_tap = false;
        }
        else
        {
            ui_controller_handle_single_tap();
        }
    }

    uint32_t position = touch_slider_get_position();
    bool is_touched = (position != UINT32_MAX);

    if (is_touched)
    {
        if (!s_is_touch_active)
        {
            ui_controller_handle_touch_start(position);
        }

        if (touch_slider_is_sliding())
        {
            ui_controller_handle_touch_slide(position);
        }

        s_last_slider_position = position;
        s_is_touch_active = true;
    }
    else if (s_is_touch_active)
    {
        ui_controller_handle_touch_release(s_last_slider_position);
        s_is_touch_active = false;
    }

    if (!s_is_touch_active && is_timer_expired(&s_release_display_hold))
    {
        ui_controller_transition_to_idle();
    }

    if (!s_is_touch_active)
    {
        uint64_t now_us = esp_timer_get_time();
        if ((now_us - s_last_interaction_time_us) / 1000 >= UI_INTERACTION_IDLE_MS)
        {
            ui_controller_transition_to_idle();
        }
    }

    if (s_ui_state == UI_STATE_IDLE)
    {
        ui_controller_handle_idle_animation();
    }
}

static void ui_controller_task(void *param)
{
    (void)param;

    while (1)
    {
        ui_controller_update();
        vTaskDelay(pdMS_TO_TICKS(UI_POLL_INTERVAL_MS));
    }
}

static void ui_controller_handle_single_tap(void)
{
    mark_interaction();

    uint32_t tap_position = touch_slider_get_first_touch_position();
    if (tap_position <= 100U)
    {
        s_last_slider_position = tap_position;
    }

    float target_snapshot;
    lock_state();
    s_ui_state = UI_STATE_TOUCHED;
    target_snapshot = s_target_temperature;
    unlock_state();

    s_last_slider_temp_c = target_snapshot;

    led_manager_show_normal(COLOR_TOUCH_WAKE.r, COLOR_TOUCH_WAKE.g, COLOR_TOUCH_WAKE.b);
    schedule_flag(&s_release_display_hold, UI_SINGLE_TAP_WAKE_HOLD_MS);
    ESP_LOGI(TAG, "Single tap detected");
}

static void ui_controller_handle_double_tap(void)
{
    mark_interaction();

    uint32_t tap_position = touch_slider_get_position();
    if (tap_position == UINT32_MAX)
    {
        tap_position = touch_slider_get_first_touch_position();
    }
    if (tap_position <= 100U)
    {
        s_last_slider_position = tap_position;
        s_last_slider_temp_c = slider_position_to_temperature(tap_position);
    }

    float target_snapshot;
    uint32_t slider_snapshot;
    lock_state();
    s_ui_state = UI_STATE_SET_CONFIRMED;
    target_snapshot = s_target_temperature;
    slider_snapshot = s_last_slider_position;
    unlock_state();

    led_manager_show_success_flash();

    float range = TEMP_MAX_CELSIUS - TEMP_MIN_CELSIUS;
    float deviation_ratio = (range > 0.0f) ? fminf(fabsf(s_last_slider_temp_c - target_snapshot) / range, 1.0f) : 0.0f;
    float emphasized = fmaxf(deviation_ratio, SLIDER_RELEASE_EMPHASIS);
    rgb_color_t slider_color = ui_controller_compute_slider_color(s_last_slider_temp_c, target_snapshot);
    slider_color = rgb_lerp(slider_color, COLOR_SUCCESS_GLOW, 0.35f);

    led_manager_show_slider_bar(slider_snapshot, emphasized, slider_color.r, slider_color.g, slider_color.b);
    schedule_flag(&s_release_display_hold, UI_POST_RELEASE_HOLD_MS + UI_DOUBLE_TAP_EXTRA_HOLD_MS);
    ESP_LOGI(TAG, "Double tap confirmed action");
}

static void ui_controller_handle_touch_start(uint32_t position)
{
    mark_interaction();

    s_slide_detected = false;

    float current_target;
    lock_state();
    s_ui_state = UI_STATE_TOUCHED;
    current_target = s_target_temperature;
    unlock_state();

    float slider_temp = slider_position_to_temperature(position);
    s_last_slider_temp_c = slider_temp;
    float delta = fabsf(slider_temp - current_target);
    float range = TEMP_MAX_CELSIUS - TEMP_MIN_CELSIUS;
    float deviation_ratio = (range > 0.0f) ? fminf(delta / range, 1.0f) : 0.0f;

    s_last_slider_position = position;
    ui_controller_render_slider(position, slider_temp, current_target, fmaxf(deviation_ratio, SLIDER_EMPHASIS_MIN));
    ESP_LOGI(TAG, "Touch start at position %u", (unsigned)position);
}

static void ui_controller_handle_touch_slide(uint32_t position)
{
    mark_interaction();

    s_slide_detected = true;

    float current_target;
    lock_state();
    s_ui_state = UI_STATE_SLIDING;
    current_target = s_target_temperature;
    unlock_state();

    float slider_temp = slider_position_to_temperature(position);
    s_last_slider_temp_c = slider_temp;
    float delta = fabsf(slider_temp - current_target);
    float range = TEMP_MAX_CELSIUS - TEMP_MIN_CELSIUS;
    float deviation_ratio = (range > 0.0f) ? fminf(delta / range, 1.0f) : 0.0f;

    s_last_slider_position = position;
    ui_controller_render_slider(position, slider_temp, current_target, deviation_ratio);
}

static void ui_controller_handle_touch_release(uint32_t position)
{
    mark_interaction();

    if (s_slide_detected)
    {
        float new_target = slider_position_to_temperature(position);

        lock_state();
        s_target_temperature = new_target;
        s_ui_state = UI_STATE_TOUCHED;
        unlock_state();

        update_setpoint(new_target);

        s_last_slider_temp_c = new_target;
        s_last_slider_position = position;

        rgb_color_t slider_color = ui_controller_compute_slider_color(new_target, new_target);
        slider_color = rgb_lerp(slider_color, COLOR_NEUTRAL_WHITE, 0.4f);

        led_manager_show_slider_bar(position, SLIDER_RELEASE_EMPHASIS, slider_color.r, slider_color.g, slider_color.b);
        schedule_flag(&s_release_display_hold, UI_POST_RELEASE_HOLD_MS);

        ESP_LOGI(TAG, "Slider released at %u -> target %.2f°C", (unsigned)position, new_target);
    }
    else
    {
        s_ignore_next_single_tap = true;
        ui_controller_handle_single_tap();
    }

    s_slide_detected = false;
}

static void ui_controller_handle_idle_animation(void)
{
    float target_snapshot;
    float current_snapshot;

    lock_state();
    target_snapshot = s_target_temperature;
    current_snapshot = s_current_temperature;
    unlock_state();

    float delta = current_snapshot - target_snapshot;
    float magnitude = fabsf(delta);
    rgb_color_t accent = (delta >= 0.0f) ? COLOR_IDLE_WARM : COLOR_IDLE_COOL;

    if (magnitude >= IDLE_ALERT_THRESHOLD)
    {
        float t = fminf((magnitude - IDLE_ALERT_THRESHOLD) / IDLE_ALERT_THRESHOLD, 1.0f);
        rgb_color_t color = rgb_lerp(accent, COLOR_IDLE_ALERT, fminf(t, 1.0f));
        led_manager_show_breathing_color(color.r, color.g, color.b, 14, 160, 4200);
    }
    else if (magnitude >= IDLE_TINT_THRESHOLD)
    {
        float t = (magnitude - IDLE_TINT_THRESHOLD) / (IDLE_ALERT_THRESHOLD - IDLE_TINT_THRESHOLD);
        rgb_color_t color = rgb_lerp(COLOR_IDLE_BASE, accent, fminf(fmaxf(t, 0.0f), 1.0f));
        led_manager_show_breathing_color(color.r, color.g, color.b, 16, 140, 6000);
    }
    else
    {
        led_manager_show_breathing_color(COLOR_IDLE_BASE.r, COLOR_IDLE_BASE.g, COLOR_IDLE_BASE.b, 16, 125, 7200);
    }
}

static void ui_controller_transition_to_idle(void)
{
    bool changed;

    lock_state();
    changed = (s_ui_state != UI_STATE_IDLE);
    s_ui_state = UI_STATE_IDLE;
    unlock_state();

    clear_flag(&s_release_display_hold);

    if (changed)
    {
        ui_controller_handle_idle_animation();
        ESP_LOGI(TAG, "UI transitioned to idle");
    }
}

static rgb_color_t rgb_lerp(rgb_color_t from, rgb_color_t to, float t)
{
    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }

    rgb_color_t result = {
        .r = (uint8_t)((1.0f - t) * (float)from.r + t * (float)to.r),
        .g = (uint8_t)((1.0f - t) * (float)from.g + t * (float)to.g),
        .b = (uint8_t)((1.0f - t) * (float)from.b + t * (float)to.b),
    };
    return result;
}

static rgb_color_t ui_controller_compute_slider_color(float slider_temp, float reference_temp)
{
    float diff = slider_temp - reference_temp;
    float range = TEMP_MAX_CELSIUS - TEMP_MIN_CELSIUS;
    float magnitude = (range > 0.0f) ? fminf(fabsf(diff) / range, 1.0f) : 0.0f;
    rgb_color_t accent = (diff >= 0.0f) ? COLOR_SLIDER_WARM : COLOR_SLIDER_COOL;
    return rgb_lerp(COLOR_SLIDER_NEUTRAL, accent, magnitude);
}

static void ui_controller_render_slider(uint32_t position, float slider_temp, float reference_temp, float deviation_ratio)
{
    float clamped = fminf(fmaxf(deviation_ratio, SLIDER_EMPHASIS_MIN), 1.0f);
    rgb_color_t color = ui_controller_compute_slider_color(slider_temp, reference_temp);

    if (deviation_ratio < SLIDER_EMPHASIS_MIN)
    {
        color = rgb_lerp(color, COLOR_NEUTRAL_WHITE, 0.3f);
    }

    led_manager_show_slider_bar(position, clamped, color.r, color.g, color.b);
}

static float slider_position_to_temperature(uint32_t position)
{
    if (position > 100)
    {
        position = 100;
    }

    float ratio = (float)position / 100.0f;
    float temperature = TEMP_MIN_CELSIUS + ratio * (TEMP_MAX_CELSIUS - TEMP_MIN_CELSIUS);
    return temperature;
}

static void mark_interaction(void)
{
    s_last_interaction_time_us = esp_timer_get_time();
}

static bool is_timer_expired(const timed_flag_t *flag)
{
    if (!flag->active)
    {
        return false;
    }

    if (flag->duration_ms == 0U)
    {
        return true;
    }

    uint64_t now = esp_timer_get_time();
    uint64_t elapsed_ms = (now - flag->timestamp_us) / 1000U;
    return elapsed_ms >= flag->duration_ms;
}

static void schedule_flag(timed_flag_t *flag, uint32_t timeout_ms)
{
    flag->active = true;
    flag->duration_ms = timeout_ms;
    flag->timestamp_us = esp_timer_get_time();
}

static void clear_flag(timed_flag_t *flag)
{
    flag->active = false;
    flag->duration_ms = 0;
    flag->timestamp_us = 0;
}
