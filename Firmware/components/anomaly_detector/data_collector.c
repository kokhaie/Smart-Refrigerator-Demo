#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "sensor_manager.h"
#include "motor_manager.h"
#include "anomaly_detector.h"
#include "data_collector.h"
#include "vibration_patterns.h"

// =====================================================
// CONFIGURATION
// =====================================================

static const char *TAG = "DATA_COLLECTOR_V2";

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

// Event GPIO for marking anomaly phases
#ifndef CONFIG_FAULT_EVENT_GPIO
#define CONFIG_FAULT_EVENT_GPIO 17
#endif

#define FAULT_EVENT_GPIO ((gpio_num_t)CONFIG_FAULT_EVENT_GPIO)

// Timing parameters (in seconds)
// *** MODIFIED FOR BALANCED 60-MINUTE SESSIONS ***
#define NORMAL_RAMP_S 180.0f   // 3 min
#define NORMAL_PID_S 1200.0f   // 20 min
#define NORMAL_WALK_S 1200.0f  // 20 min
#define NORMAL_STATIC_S 200.0f // 10 min (5x 2min)
// TOTAL NORMAL: ~53 min + cooldowns (~60 min)

#define BEARING_RAMP_PER_AMP_S 1200.0f // 20 min x 3 amps
// TOTAL BEARING: 60 min

#define IMBALANCE_PER_SPEED_S 240.0f // 4 min x 5 speeds x 3 amps
// TOTAL IMBALANCE: 60 min

#define ELECTRICAL_WALK_PER_AMP_S 1200.0f // 20 min x 3 amps
// TOTAL ELECTRICAL: 60 min

#define COOLDOWN_DURATION_S 10.0f // Between phases

// Fan speed limits
#define FAN_MIN_SPEED 0
#define FAN_MAX_SPEED 100

typedef struct
{
    collection_mode_t mode;
    const char *mode_name;
    uint8_t event_code; // For GPIO marking
} collection_config_t;

// =====================================================
// GLOBAL STATE
// =====================================================

static volatile uint8_t s_current_fan_pwm = 0;
static volatile uint8_t s_current_vib_pwm = 0;
static volatile uint8_t s_event_code = 0;
static collection_config_t s_config;

// =====================================================
// UTILITY FUNCTIONS
// =====================================================

static inline uint8_t clamp_speed(int value)
{
    if (value < 0)
        return 0;
    if (value > 100)
        return 100;
    return (uint8_t)value;
}

static void delay_ms(int ms)
{
    if (ms <= 0)
        ms = 1;
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void fault_gpio_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << FAULT_EVENT_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&cfg);
    gpio_set_level(FAULT_EVENT_GPIO, 0);
}

static inline void set_event_marker(uint8_t code)
{
    s_event_code = code;
    gpio_set_level(FAULT_EVENT_GPIO, code ? 1 : 0);
}

static void apply_fan_speed(uint8_t speed_percent)
{
    s_current_fan_pwm = clamp_speed(speed_percent);
    set_fan_speed(s_current_fan_pwm);
    anomaly_detector_record_pwm(s_current_fan_pwm);
}

void apply_vibration_speed(uint8_t speed_percent)
{
    s_current_vib_pwm = clamp_speed(speed_percent);
    set_vibration_speed(s_current_vib_pwm);
}

// =====================================================
// VIBRATION MOTOR ANOMALY PATTERNS
// =====================================================

//
// [PATTERN 1 - NEW]: Bearing wear - "grindy" high-frequency + long spikes
//
static int s_spike_counter = 0; // Spike duration counter
void vibration_pattern_bearing_wear(float amplitude_0_to_1, int duration_ms)
{
    const int step_ms = 20; // 50 Hz update rate
    float t = 0.0f;

    for (int elapsed = 0; elapsed < duration_ms; elapsed += step_ms)
    {
        // 1. High-frequency "grinding" sine wave
        float base_freq_hz = 85.0f; // Fast, metallic grind
        float phase = 2.0f * PI_F * base_freq_hz * t;
        float base_vib = 0.5f + (0.5f * sinf(phase)); // 0.0 to 1.0

        // 2. Modulate with low-frequency random noise to make it "grindy"
        float noise = 0.5f + ((float)(esp_random() % 100) / 200.0f); // 0.5 to 1.0
        float value = amplitude_0_to_1 * base_vib * noise;           // Scale by noise and amplitude

        // 3. Add random "spikes" (NEW: multi-step spikes)
        if (s_spike_counter > 0)
        {
            // Continue spike
            value = amplitude_0_to_1;
            s_spike_counter--;
        }
        else if ((esp_random() % 100) < 10) // 10% chance of a *new* spike
        {
            value = amplitude_0_to_1; // Full amplitude spike
            s_spike_counter = 2;      // Spike lasts 3 steps (20ms * 3 = 60ms)
        }
        // else: no spike, just grinding

        uint8_t vib_pwm = (uint8_t)(value * 100.0f);
        apply_vibration_speed(vib_pwm);

        delay_ms(step_ms);
        t += (step_ms / 1000.0f);
    }
}

//
// [PATTERN 2 - UNCHANGED]: Rotor imbalance - frequency scales with fan speed
//
void vibration_pattern_imbalance(uint8_t fan_speed, float amplitude_0_to_1, int duration_ms)
{
    // *** CALIBRATION NEEDED ***
    // You must calibrate this 50.0f value to your actual motor!
    float MAX_MOTOR_FREQ_HZ = 50.0f;
    float rotation_hz = (fan_speed / 100.0f) * MAX_MOTOR_FREQ_HZ;

    const int step_ms = 20;
    float t = 0.0f;

    for (int elapsed = 0; elapsed < duration_ms; elapsed += step_ms)
    {
        float phase = 2.0f * PI_F * rotation_hz * t;
        float value = 0.4f + (amplitude_0_to_1 * 0.6f * sinf(phase));

        uint8_t vib_pwm = (uint8_t)(value * 100.0f);
        apply_vibration_speed(vib_pwm);

        delay_ms(step_ms);
        t += (step_ms / 1000.0f);
    }
}

//
// [PATTERN 3 - NEW]: Electrical fault - "flickering" 1Hz square wave
//
void vibration_pattern_electrical(float amplitude_0_to_1, int duration_ms)
{
    const int step_ms = 500; // 1Hz cycle (500ms on, 500ms off)
    bool on = true;
    uint8_t vib_pwm = (uint8_t)(amplitude_0_to_1 * 100.0f);

    for (int elapsed = 0; elapsed < duration_ms; elapsed += step_ms)
    {
        if (on)
        {
            apply_vibration_speed(vib_pwm);
        }
        else
        {
            apply_vibration_speed(0);
        }
        on = !on; // Flip the state

        vTaskDelay(pdMS_TO_TICKS(step_ms));
    }
}

// =====================================================
// SPEED CONTROL PATTERNS
// =====================================================

// Pattern A: Slow linear ramp
static void speed_pattern_slow_ramp(bool ramp_up, float duration_s)
{
    int duration_ms = (int)(duration_s * 1000.0f);
    const int step_ms = 100;
    int64_t start_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Slow ramp %s: %.1f seconds", ramp_up ? "UP" : "DOWN", duration_s);

    while (true)
    {
        int64_t now = esp_timer_get_time();
        int elapsed_ms = (int)((now - start_time) / 1000);
        if (elapsed_ms >= duration_ms)
            break;
        float progress = (float)elapsed_ms / (float)duration_ms;
        uint8_t speed = ramp_up ? (FAN_MIN_SPEED + (uint8_t)(progress * (FAN_MAX_SPEED - FAN_MIN_SPEED)))
                                : (FAN_MAX_SPEED - (uint8_t)(progress * (FAN_MAX_SPEED - FAN_MIN_SPEED)));
        apply_fan_speed(speed);
        delay_ms(step_ms);
    }
}

// Pattern B: PID-like behavior
static void speed_pattern_pid_simulation(float duration_s)
{
    int duration_ms = (int)(duration_s * 1000.0f);
    const int step_ms = 500;
    uint8_t target_speed = 60, current_speed = target_speed;
    int64_t start_time = esp_timer_get_time();
    int disturbance_timer = 0;
    ESP_LOGI(TAG, "PID simulation: %.1f seconds", duration_s);

    while (true)
    {
        int64_t now = esp_timer_get_time();
        int elapsed_ms = (int)((now - start_time) / 1000);
        if (elapsed_ms >= duration_ms)
            break;

        disturbance_timer += step_ms;
        if (disturbance_timer >= 60000)
        {
            ESP_LOGI(TAG, "Simulated disturbance - increasing load");
            target_speed = 50 + (esp_random() % 40); // Random setpoint 50-90%
            disturbance_timer = 0;
        }

        int error = target_speed - current_speed;
        current_speed = clamp_speed(current_speed + (error / 2));
        apply_fan_speed(current_speed);
        delay_ms(step_ms);
    }
}

// Pattern C: Random walk
static void speed_pattern_random_walk(float duration_s)
{
    int duration_ms = (int)(duration_s * 1000.0f);
    const int step_ms = 1000;
    uint8_t current_speed = (FAN_MIN_SPEED + FAN_MAX_SPEED) / 2;
    int64_t start_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Random walk: %.1f seconds", duration_s);

    while (true)
    {
        int64_t now = esp_timer_get_time();
        int elapsed_ms = (int)((now - start_time) / 1000);
        if (elapsed_ms >= duration_ms)
            break;

        int delta = ((int)(esp_random() % 21)) - 10; // -10 to +10
        current_speed = clamp_speed(current_speed + delta);
        apply_fan_speed(current_speed);
        delay_ms(step_ms);
    }
}

// Pattern D: Static speeds
static void speed_pattern_static_levels(float hold_duration_s)
{
    uint8_t test_speeds[] = {30, 50, 70, 90, 100};
    int num_speeds = sizeof(test_speeds) / sizeof(test_speeds[0]);
    ESP_LOGI(TAG, "Static levels: %d speeds, %.1f seconds each", num_speeds, hold_duration_s);

    for (int i = 0; i < num_speeds; i++)
    {
        ESP_LOGI(TAG, "Static speed: %d%%", test_speeds[i]);
        apply_fan_speed(test_speeds[i]);
        delay_ms((int)(hold_duration_s * 1000.0f));
    }
}

// =====================================================
// NOISE INJECTION (FOR NORMAL CLASS)
// =====================================================

static void inject_external_noise_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "External noise injection task started");

    while (true)
    {
        int wait_s = 10 + (esp_random() % 20);
        vTaskDelay(pdMS_TO_TICKS(wait_s * 1000));

        if (s_config.mode == MODE_NORMAL)
        {
            ESP_LOGI(TAG, "Injecting simulated desk tap");
            apply_vibration_speed(80);
            delay_ms(50); // Very brief
            apply_vibration_speed(0);
        }
    }
}

// =====================================================
// MAIN COLLECTION SEQUENCES
// =====================================================

// Sequence 1: NORMAL data collection (~60 min)
static void collect_normal_data(void)
{
    ESP_LOGI(TAG, "COLLECTING NORMAL DATA (~60 min)");
    set_event_marker(0);
    apply_vibration_speed(0);

    ESP_LOGI(TAG, "Phase 1: Slow ramp up/down (6 min)");
    speed_pattern_slow_ramp(true, NORMAL_RAMP_S);
    delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));
    speed_pattern_slow_ramp(false, NORMAL_RAMP_S);
    delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));

    ESP_LOGI(TAG, "Phase 2: PID simulation (20 min)");
    speed_pattern_pid_simulation(NORMAL_PID_S);
    delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));

    ESP_LOGI(TAG, "Phase 3: Random walk (20 min)");
    speed_pattern_random_walk(NORMAL_WALK_S);
    delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));

    ESP_LOGI(TAG, "Phase 4: Static validation levels (10 min)");
    speed_pattern_static_levels(NORMAL_STATIC_S);

    apply_fan_speed(0);
    ESP_LOGI(TAG, "NORMAL data collection complete");
}

// Sequence 2: BEARING_WEAR data collection (~60 min)
static void collect_bearing_wear_data(void)
{
    ESP_LOGI(TAG, "COLLECTING BEARING WEAR DATA (~60 min)");
    set_event_marker(1);

    float amplitudes[] = {0.3f, 0.5f, 0.7f};

    for (int amp_idx = 0; amp_idx < 3; amp_idx++)
    {
        float amplitude = amplitudes[amp_idx];
        ESP_LOGI(TAG, "Bearing wear amplitude: %.1f (20 min)", amplitude);

        // ** FIX APPLIED: Run pattern continuously during ramp **

        ESP_LOGI(TAG, "Ramp up with bearing pattern");
        int ramp_duration_ms = (int)(BEARING_RAMP_PER_AMP_S * 1000.0f / 2.0f); // Half up, half down
        int64_t start = esp_timer_get_time();

        while (true)
        {
            int64_t now = esp_timer_get_time();
            int elapsed_ms = (int)((now - start) / 1000);
            if (elapsed_ms >= ramp_duration_ms)
                break;

            float progress = (float)elapsed_ms / (float)ramp_duration_ms;
            uint8_t speed = FAN_MIN_SPEED + (uint8_t)(progress * (FAN_MAX_SPEED - FAN_MIN_SPEED));
            apply_fan_speed(speed);

            // Run continuously for 1s (matches model window)
            vibration_pattern_bearing_wear(amplitude, 1000);
        }

        ESP_LOGI(TAG, "Ramp down with bearing pattern");
        start = esp_timer_get_time();

        while (true)
        {
            int64_t now = esp_timer_get_time();
            int elapsed_ms = (int)((now - start) / 1000);
            if (elapsed_ms >= ramp_duration_ms)
                break;

            float progress = (float)elapsed_ms / (float)ramp_duration_ms;
            uint8_t speed = FAN_MAX_SPEED - (uint8_t)(progress * (FAN_MAX_SPEED - FAN_MIN_SPEED));
            apply_fan_speed(speed);

            // Run continuously for 1s
            vibration_pattern_bearing_wear(amplitude, 1000);
        }

        delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));
    }

    apply_fan_speed(0);
    apply_vibration_speed(0);
    set_event_marker(0);
    ESP_LOGI(TAG, "BEARING WEAR data collection complete");
}

// Sequence 3: ROTOR_IMBALANCE data collection (~60 min)
static void collect_imbalance_data(void)
{
    ESP_LOGI(TAG, "COLLECTING ROTOR IMBALANCE DATA (~60 min)");
    set_event_marker(2);

    float amplitudes[] = {0.4f, 0.6f, 0.8f};

    for (int amp_idx = 0; amp_idx < 3; amp_idx++)
    {
        float amplitude = amplitudes[amp_idx];
        ESP_LOGI(TAG, "Imbalance amplitude: %.1f (20 min total)", amplitude);

        uint8_t test_speeds[] = {40, 55, 70, 85, 100};

        for (int spd_idx = 0; spd_idx < 5; spd_idx++)
        {
            uint8_t speed = test_speeds[spd_idx];
            ESP_LOGI(TAG, "Speed: %d%% with imbalance (4 min)", speed);
            apply_fan_speed(speed);
            delay_ms(2000); // Stabilize

            // Run imbalance pattern for 4 minutes
            vibration_pattern_imbalance(speed, amplitude, (int)(IMBALANCE_PER_SPEED_S * 1000.0f));
            delay_ms(2000); // Pause
        }

        delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));
    }

    apply_fan_speed(0);
    apply_vibration_speed(0);
    set_event_marker(0);
    ESP_LOGI(TAG, "ROTOR IMBALANCE data collection complete");
}

// Sequence 4: ELECTRICAL_FAULT data collection (~60 min)
static void collect_electrical_data(void)
{
    ESP_LOGI(TAG, "COLLECTING ELECTRICAL FAULT DATA (~60 min)");
    set_event_marker(3);

    float amplitudes[] = {0.5f, 0.7f, 0.9f};

    for (int amp_idx = 0; amp_idx < 3; amp_idx++)
    {
        float amplitude = amplitudes[amp_idx];
        ESP_LOGI(TAG, "Electrical fault amplitude: %.1f (20 min)", amplitude);

        int duration_ms = (int)(ELECTRICAL_WALK_PER_AMP_S * 1000.0f);
        int64_t start = esp_timer_get_time();
        uint8_t current_speed = 60;

        while (true)
        {
            int64_t now = esp_timer_get_time();
            int elapsed_ms = (int)((now - start) / 1000);
            if (elapsed_ms >= duration_ms)
                break;

            // Random walk fan speed
            int delta = ((int)(esp_random() % 21)) - 10;
            current_speed = clamp_speed(current_speed + delta);
            apply_fan_speed(current_speed);

            // ** FIX APPLIED: Run for exactly 1000ms (2 cycles) **
            vibration_pattern_electrical(amplitude, 1000);
        }

        delay_ms((int)(COOLDOWN_DURATION_S * 1000.0f));
    }

    apply_fan_speed(0);
    apply_vibration_speed(0);
    set_event_marker(0);
    ESP_LOGI(TAG, "ELECTRICAL FAULT data collection complete");
}

// =====================================================
// LOGGING TASK
// =====================================================

static void logging_task(void *arg)
{
    (void)arg;
    uint64_t session_start = 0;
    printf("# Collection Mode: %s\n", s_config.mode_name);
    printf("# Columns: time_ms,accel_mag_g,fan_pwm,vibration_pwm,current_a,event_code\n");

    while (true)
    {
        synchronized_sample_t sample;
        if (!sensor_manager_get_raw_sample(&sample, portMAX_DELAY))
            continue;
        if (session_start == 0)
            session_start = sample.timestamp_us;
        uint32_t t_ms = (uint32_t)((sample.timestamp_us - session_start) / 1000ULL);

        printf("%lu,%.5f,%u,%u,%.3f,%u\n",
               (unsigned long)t_ms,
               sample.magnitude,
               (unsigned)s_current_fan_pwm,
               (unsigned)s_current_vib_pwm,
               sample.latest_current_a,
               (unsigned)s_event_code);
    }
}

// =====================================================
// MAIN COLLECTION TASK
// =====================================================

static void collection_task(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "Waiting 5 seconds for sensor stabilization...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI(TAG, "Starting data collection in mode: %s", s_config.mode_name);

    switch (s_config.mode)
    {
    case MODE_NORMAL:
        collect_normal_data();
        break;
    case MODE_BEARING_WEAR:
        collect_bearing_wear_data();
        break;
    case MODE_ROTOR_IMBALANCE:
        collect_imbalance_data();
        break;
    case MODE_ELECTRICAL:
        collect_electrical_data();
        break;
    default:
        ESP_LOGE(TAG, "Unknown collection mode: %d", s_config.mode);
        break;
    }

    apply_fan_speed(0);
    apply_vibration_speed(0);
    set_event_marker(0);
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "DATA COLLECTION COMPLETE");
    ESP_LOGI(TAG, "========================================");
    vTaskDelete(NULL);
}

void data_collector_start(collection_mode_t mode)
{
    setvbuf(stdout, NULL, _IONBF, 0); // Disable buffering for CSV
    
    fault_gpio_init();
    apply_fan_speed(0);
    apply_vibration_speed(0);
    set_event_marker(0);

    s_config.mode = mode;

    switch (s_config.mode)
    {
    case MODE_NORMAL:
        s_config.mode_name = "NORMAL";
        s_config.event_code = 0;
        break;
    case MODE_BEARING_WEAR:
        s_config.mode_name = "BEARING_WEAR";
        s_config.event_code = 1;
        break;
    case MODE_ROTOR_IMBALANCE:
        s_config.mode_name = "ROTOR_IMBALANCE";
        s_config.event_code = 2;
        break;
    case MODE_ELECTRICAL:
        s_config.mode_name = "ELECTRICAL";
        s_config.event_code = 3;
        break;
    default:
        s_config.mode_name = "UNKNOWN";
        s_config.event_code = 0;
        break;
    }

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "DATA COLLECTION SYSTEM V2 (Balanced)");
    ESP_LOGI(TAG, "Mode: %s", s_config.mode_name);
    ESP_LOGI(TAG, "========================================");

    xTaskCreatePinnedToCore(logging_task, "csv_logger", 6144, NULL, 9, NULL, 0);

    if (s_config.mode == MODE_NORMAL)
    {
        xTaskCreatePinnedToCore(inject_external_noise_task, "noise_inject", 4096, NULL, 5, NULL, 0);
    }

    xTaskCreatePinnedToCore(collection_task, "collector", 8192, NULL, 8, NULL, 1);
    
    ESP_LOGI(TAG, "All tasks started successfully");
}
