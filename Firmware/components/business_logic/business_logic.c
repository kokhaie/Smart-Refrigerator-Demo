#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "motor_manager.h"
#include "business_logic.h"
#include "sensor_manager.h"

static const char *TAG = "BUSINESS_LOGIC";

// ------------------- PID Tuning Constants -------------------
float Kp = 99.0f;
float Ki = 0.33f;
float Kd = 0.0f;
float setpoint = 10.0f;

// ------------------- Control Parameters -------------------
#define TEMP_BAND 0.5f       // °C - Hysteresis band
#define INTEGRAL_DECAY 0.98f // Gradual integral decay
#define DUTY_CUTOFF_OFF 5    // Below this value, compressor turns OFF
#define DUTY_CUTOFF_ON 12    // At/above this value, compressor turns ON

// Ramping - gradual changes
#define DUTY_RAMP_UP 8.0f   // Fast ramp up (% per cycle)
#define DUTY_RAMP_DOWN 3.0f // Slow ramp down (% per cycle)

// ------------------- Simulation Constants -------------------
#define USE_REAL_SENSOR 0        // 0 = simulation only | 1 = use real sensor
#define SIM_AMBIENT_C 25.0f      // Ambient (room) temperature
#define SIM_HEAT_LEAK_RATE 0.02f // Warming from ambient
#define SIM_MAX_COOL_RATE 0.85f  // Max cooling rate
#define SIM_THERMAL_MASS 18.0f   // Thermal mass (resistance to temp change)
#define SIM_MIN_C 0.0f           // Minimum temperature
#define SIM_MAX_C 50.0f          // Maximum temperature
#define SIM_INITIAL_TEMP 25.0f   // Initial simulated temperature

// ------------------- Globals -------------------
QueueHandle_t g_setpoint_queue;
static float sim_temp_c = SIM_INITIAL_TEMP; // Simulated temperature

// Hysteresis state
static bool fan_on = false;  // Fan starts OFF
static bool duty_on = false; // Duty starts OFF

// Ramped duty state
static float current_duty = 0.0f;

// ------------------- Simulation Update -------------------
static void sim_update(float fan_percent, float dt_sec, float ambient_c)
{
    // Cooling power based on fan percentage
    float cooling_power = (fan_percent / 100.0f) * SIM_MAX_COOL_RATE;

    // Heat leak from ambient (the warmer the ambient, the more it warms)
    float heat_leak = (ambient_c - sim_temp_c) * SIM_HEAT_LEAK_RATE;

    // Temperature change = (heat from ambient - fan cooling) / thermal mass
    float dT = (heat_leak - cooling_power) / SIM_THERMAL_MASS;

    // Apply temperature change
    sim_temp_c += dT * dt_sec;

    // Clamp temperature to allowed range
    if (sim_temp_c < SIM_MIN_C)
        sim_temp_c = SIM_MIN_C;
    if (sim_temp_c > SIM_MAX_C)
        sim_temp_c = SIM_MAX_C;
}

void update_setpoint(float new_setpoint)
{
    if (g_setpoint_queue != NULL)
    {
        xQueueOverwrite(g_setpoint_queue, &new_setpoint);
    }
    else
    {
        setpoint = new_setpoint;
        ESP_LOGW(TAG, "Setpoint queue not ready; applied setpoint %.2f°C directly", new_setpoint);
    }
}

// ------------------- PID Task -------------------
void pid_fan_control_task(void *pvParameters)
{
    shtc3_data_t shtc3_data;

    float integral_term = 0.0f;
    float previous_process_variable = SIM_INITIAL_TEMP;

    int64_t last_time = esp_timer_get_time();
    float last_fan_percent = 0.0f;

    ESP_LOGI(TAG, "🚀 PID controller started - Simulation mode");
    ESP_LOGI(TAG, "📊 Settings: Kp=%.1f, Ki=%.2f, Kd=%.1f", Kp, Ki, Kd);
    ESP_LOGI(TAG, "🎯 Setpoint: %.1f°C | Initial temp: %.1f°C", setpoint, sim_temp_c);

    for (;;)
    {
        // --- Receive setpoint updates ---
        float received_setpoint;
        if (xQueueReceive(g_setpoint_queue, &received_setpoint, 0) == pdTRUE)
        {
            ESP_LOGI(TAG, "🎯 New setpoint: %.1f°C", received_setpoint);
            setpoint = received_setpoint;
            integral_term = 0.0f; // Reset integral term
        }

        // --- Read sensor (for display only - not used in simulation mode) ---
        bool sensor_ok = sensor_manager_get_latest_environment(&shtc3_data);

        // --- Compute elapsed time ---
        int64_t now = esp_timer_get_time();
        float time_delta = (float)(now - last_time) / 1000000.0f;
        last_time = now;
        if (time_delta <= 0.0f || time_delta > 5.0f)
        {
            time_delta = 0.25f; // Fallback default
        }

        // --- Update simulation ---
        sim_update(last_fan_percent, time_delta, SIM_AMBIENT_C);

        // --- Select Process Variable ---
        float process_variable;

#if USE_REAL_SENSOR
        // Use the real sensor
        if (sensor_ok)
        {
            process_variable = shtc3_data.temperature_c;
        }
        else
        {
            process_variable = sim_temp_c; // If sensor fails, fall back to simulation
        }
#else
        // Use simulation (demo mode)
        process_variable = sim_temp_c;
#endif

        // ------------------- PID Logic -------------------

        // Error: current temp - desired temp
        float error = process_variable - setpoint;

        // P term (proportional)
        float p_term = Kp * error;

        // I term (integral) with gradual decay
        float u_unsat = p_term + integral_term;
        if (error > 0.0f)
        {
            // If too warm (positive error), accumulate integral
            if (!(u_unsat > 255.0f))
            {
                integral_term += Ki * error * time_delta;
            }
        }
        else
        {
            // If too cold (negative error), decay integral
            integral_term *= INTEGRAL_DECAY;
        }
        // Clamp integral
        if (integral_term < 0.0f)
            integral_term = 0.0f;
        if (integral_term > 255.0f)
            integral_term = 255.0f;

        // D term (derivative)
        float derivative = (process_variable - previous_process_variable) / time_delta;
        previous_process_variable = process_variable;
        float d_term = Kd * derivative;

        // Total PID output
        float output = p_term + integral_term + d_term;
        if (output < 0.0f)
            output = 0.0f;
        if (output > 255.0f)
            output = 255.0f;

        // Convert to percentage
        float duty_pct = (output / 255.0f) * 100.0f;

        // ------------------- Temperature hysteresis -------------------
        // Prevent rapid ON/OFF toggling

        if (fan_on)
        {
            // Fan is ON - turn it OFF if we drop below (setpoint - band)
            if (process_variable <= (setpoint - TEMP_BAND))
            {
                fan_on = false;
                ESP_LOGI(TAG, "❄️ Temperature reached %.2f°C - Fan turned OFF", process_variable);
            }
        }
        else
        {
            // Fan is OFF - turn it ON if we rise above setpoint
            if (process_variable >= setpoint)
            {
                fan_on = true;
                ESP_LOGI(TAG, "🔥 Temperature reached %.2f°C - Fan turned ON", process_variable);
            }
        }

        // ------------------- Duty cutoff hysteresis -------------------
        // Prevent compressor from running at very low duty

        if (duty_on)
        {
            if (duty_pct < DUTY_CUTOFF_OFF)
            {
                duty_on = false;
                ESP_LOGI(TAG, "⚡ Duty dropped to %.1f%% - Compressor OFF", duty_pct);
            }
        }
        else
        {
            if (duty_pct >= DUTY_CUTOFF_ON)
            {
                duty_on = true;
                ESP_LOGI(TAG, "⚡ Duty rose to %.1f%% - Compressor ON", duty_pct);
            }
        }

        // Compute target duty
        float target_duty = (fan_on && duty_on) ? duty_pct : 0.0f;

        // ------------------- Asymmetric Ramping -------------------
        // The fan shouldn't jump instantly from 0 to 100; change gradually

        if (target_duty > current_duty + DUTY_RAMP_UP)
        {
            // Need to go up - increase quickly
            current_duty += DUTY_RAMP_UP;
        }
        else if (target_duty < current_duty - DUTY_RAMP_DOWN)
        {
            // Need to go down - decrease slowly
            current_duty -= DUTY_RAMP_DOWN;
        }
        else
        {
            // Close to target - snap to it
            current_duty = target_duty;
        }

        // Clamp duty to 0–100%
        if (current_duty > 100.0f)
            current_duty = 100.0f;
        if (current_duty < 0.0f)
            current_duty = 0.0f;

        // ------------------- Apply to hardware -------------------
        set_fan_speed((uint8_t)current_duty);
        last_fan_percent = current_duty;

        // ------------------- Status output -------------------
        ESP_LOGI(TAG,
                 "🎯SP:%.1f°C | 🌡️Sim:%.2f°C | ❌Err:%.2f°C | "
                 "🔧P:%.0f I:%.0f | 🎚️Target:%.1f%% | 🌀Fan:%.1f%% | "
                 "💨On:%d ⚡Duty:%d | 📡Real:%s%.2f°C",
                 setpoint,
                 process_variable,
                 error,
                 p_term, integral_term,
                 target_duty,
                 current_duty,
                 fan_on, duty_on,
                 sensor_ok ? "" : "❌",
                 sensor_ok ? shtc3_data.temperature_c : NAN);

        vTaskDelay(pdMS_TO_TICKS(250)); // Every 250 ms (4 Hz)
    }
}

void business_logic_start(void)
{
    g_setpoint_queue = xQueueCreate(1, sizeof(float));
    xTaskCreatePinnedToCore(pid_fan_control_task, "pid_fan_control",
                            4096, NULL, 5, NULL, 1);
}
