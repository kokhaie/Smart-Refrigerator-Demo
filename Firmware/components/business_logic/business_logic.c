#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "motors.h"
#include "business_logic.h"
#include "sensor_manager.h"

static const char *TAG = "BUSINESS_LOGIC";

// ------------------- PID Tuning Constants -------------------
float Kp = 99.0f;
float Ki = 0.33f;
float Kd = 0.0f;
float setpoint = 10.0f;

// ------------------- Control Parameters -------------------
#define TEMP_BAND        0.5f   // °C hysteresis around setpoint
#define INTEGRAL_DECAY   0.98f  // bleed integrator when below SP
#define DUTY_CUTOFF_OFF  5      // below this, compressor OFF
#define DUTY_CUTOFF_ON   12     // above this, compressor ON again

// Ramping
#define DUTY_RAMP_UP     8.0f   // % per control cycle (fast increase)
#define DUTY_RAMP_DOWN   3.0f   // % per control cycle (slow decrease)

// ------------------- Simulation Constants -------------------
#define USE_SENSOR_AMBIENT 0
#define SIM_AMBIENT_C      30.0f
#define SIM_HEAT_LEAK_RATE 0.015f
#define SIM_MAX_COOL_RATE  0.80f
#define SIM_THERMAL_MASS   20.0f
#define SIM_MIN_C          0.0f
#define SIM_MAX_C          60.0f

// ------------------- Globals -------------------
QueueHandle_t g_setpoint_queue;
static float sim_temp_c = SIM_AMBIENT_C;
static bool sim_seeded = false;

// Hysteresis state
static bool fan_on = true;
static bool duty_on = true;

// Ramped duty state
static float current_duty = 0.0f;

// ------------------- Simulation Helpers -------------------
static void sim_seed_from_sensor(float sensor_temp_c, bool sensor_ok) {
    if (sensor_ok && sensor_temp_c > SIM_MIN_C && sensor_temp_c < SIM_MAX_C) {
        sim_temp_c = sensor_temp_c;
    } else {
        sim_temp_c = SIM_AMBIENT_C;
    }
    sim_seeded = true;
}

static void sim_update(float fan_percent, float dt_sec, float ambient_c) {
    float cooling_power = (fan_percent / 100.0f) * SIM_MAX_COOL_RATE;
    float heat_leak = (ambient_c - sim_temp_c) * SIM_HEAT_LEAK_RATE;
    float dT = (heat_leak - cooling_power) / SIM_THERMAL_MASS;
    sim_temp_c += dT * dt_sec;

    if (sim_temp_c < SIM_MIN_C) sim_temp_c = SIM_MIN_C;
    if (sim_temp_c > SIM_MAX_C) sim_temp_c = SIM_MAX_C;
}

void update_setpoint(float new_setpoint) {
    xQueueOverwrite(g_setpoint_queue, &new_setpoint);
}

// ------------------- PID Task -------------------
void pid_fan_control_task(void *pvParameters) {
    shtc3_data_t shtc3_data;

    float integral_term = 0.0f;
    float previous_process_variable = SIM_AMBIENT_C;

    int64_t last_time = esp_timer_get_time();
    float last_fan_percent = 0.0f;

    for (;;) {
        // --- Handle setpoint updates ---
        float received_setpoint;
        if (xQueueReceive(g_setpoint_queue, &received_setpoint, 0) == pdTRUE) {
            setpoint = received_setpoint;
            integral_term = 0.0f; // reset integrator on setpoint change
        }

        // --- Sensor update ---
        bool sensor_ok = sensor_manager_get_latest_environment(&shtc3_data);

        // --- Timing ---
        int64_t now = esp_timer_get_time();
        float time_delta = (float)(now - last_time) / 1000000.0f;
        last_time = now;
        if (time_delta <= 0.0f || time_delta > 5.0f) {
            time_delta = 0.25f; // fallback
        }

        // --- Simulation update ---
        if (!sim_seeded) {
            sim_seed_from_sensor(sensor_ok ? shtc3_data.temperature_c : SIM_AMBIENT_C,
                                 sensor_ok);
        }
        float ambient_value = (USE_SENSOR_AMBIENT && sensor_ok) ?
                              shtc3_data.temperature_c : SIM_AMBIENT_C;
        sim_update(last_fan_percent, time_delta, ambient_value);

        float process_variable = sim_temp_c;

        // ------------------- PID Logic -------------------
        float error = process_variable - setpoint;

        // P term
        float p_term = Kp * error;

        // I term with decay
        float u_unsat = p_term + integral_term;
        if (error > 0.0f) {
            if (!(u_unsat > 255.0f && error > 0.0f)) {
                integral_term += Ki * error * time_delta;
            }
        } else {
            integral_term *= INTEGRAL_DECAY;
        }
        if (integral_term < 0.0f) integral_term = 0.0f;
        if (integral_term > 255.0f) integral_term = 255.0f;

        // D term
        float derivative = (process_variable - previous_process_variable) / time_delta;
        previous_process_variable = process_variable;
        float d_term = Kd * derivative;

        // Raw PID output
        float output = p_term + integral_term + d_term;
        if (output < 0.0f) output = 0.0f;
        if (output > 255.0f) output = 255.0f;
        float duty_pct = (output / 255.0f) * 100.0f;

        // ------------------- Temperature hysteresis -------------------
        if (fan_on) {
            if (process_variable <= setpoint) {
                fan_on = false;
            }
        } else {
            if (process_variable >= setpoint + TEMP_BAND) {
                fan_on = true;
            }
        }

        // ------------------- Duty cutoff hysteresis -------------------
        if (duty_on) {
            if (duty_pct < DUTY_CUTOFF_OFF) {
                duty_on = false;
            }
        } else {
            if (duty_pct >= DUTY_CUTOFF_ON) {
                duty_on = true;
            }
        }

        float target_duty = (fan_on && duty_on) ? duty_pct : 0.0f;

        // ------------------- Asymmetric Ramping -------------------
        if (target_duty > current_duty + DUTY_RAMP_UP) {
            current_duty += DUTY_RAMP_UP;
        } else if (target_duty < current_duty - DUTY_RAMP_DOWN) {
            current_duty -= DUTY_RAMP_DOWN;
        } else {
            current_duty = target_duty;
        }

        if (current_duty > 100.0f) current_duty = 100.0f;
        if (current_duty < 0.0f) current_duty = 0.0f;

        // Apply to hardware
        set_fan_speed((uint8_t)current_duty);
        last_fan_percent = current_duty;

        ESP_LOGI(TAG, "SP: %.2f°C | Real: %.2f°C | Sim(PV): %.2f°C | TargetFan: %.1f%% | RampedFan: %.1f%%",
                 setpoint,
                 sensor_ok ? shtc3_data.temperature_c : NAN,
                 process_variable,
                 target_duty,
                 current_duty);

        vTaskDelay(pdMS_TO_TICKS(250)); // ~4 Hz
    }
}

void business_logic_start(void) {
    g_setpoint_queue = xQueueCreate(1, sizeof(float));
    xTaskCreatePinnedToCore(pid_fan_control_task, "pid_fan_control",
                            4096, NULL, 5, NULL, 1);
}
