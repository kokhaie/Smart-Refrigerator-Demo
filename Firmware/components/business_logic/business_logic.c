#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "motors.h"
#include "business_logic.h"
#include "sensor_manager.h"

static const char *TAG = "BUSINESS_LOGIC";

// PID Tuning Constants ---
float Kp = 2.5f;
float Ki = 0.5f;
float Kd = 1.0f;
float setpoint = 25.0f;
QueueHandle_t g_setpoint_queue;

void update_setpoint(float new_stepoint)
{
    xQueueOverwrite(g_setpoint_queue, &new_stepoint);
}

// --- 3. PID Control Task ---
void pid_fan_control_task(void *pvParameters)
{

    float setpoint = 25.0f;
    float received_setpoint;

    shtc3_data_t shtc3_data;

    // PID state variables
    float integral_term = 0.0f; // This will hold the complete integral term
    float previous_process_variable = 0.0f;
    bool is_first_run = true; // Flag to handle the first loop iteration correctly

    // Timekeeping variables
    int64_t last_time = esp_timer_get_time();

    // --- Main Control Loop ---
    for (;;)
    {
        if (xQueueReceive(g_setpoint_queue, &received_setpoint, 0) == pdTRUE)
            setpoint = received_setpoint;

        if (sensor_manager_read_shtc3(&shtc3_data) == ESP_OK)
        {
            // --- Time Calculation ---
            int64_t now = esp_timer_get_time();
            float time_delta = (float)(now - last_time) / 1000000.0f; // Time delta in seconds
            last_time = now;

            float process_variable = shtc3_data.temperature_c;

            // FIX 1: Prevent large derivative "kick" on the first run
            if (is_first_run)
            {
                previous_process_variable = process_variable;
                is_first_run = false;
                // Skip the rest of the loop for the first valid reading
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }

            // --- PID Calculation ---
            float error = process_variable - setpoint;
            float p_term = Kp * error;

            // The derivative is calculated before the integral for use in anti-windup
            float d_term = Kd * (process_variable - previous_process_variable) / time_delta;
            previous_process_variable = process_variable;

            // FIX 2: Implement robust anti-windup (Conditional Integration)
            float potential_output = p_term + integral_term + d_term;
            if (potential_output < 255.0f && potential_output > 0.0f)
            {
                integral_term += Ki * error * time_delta;
            }

            // --- Final Output Calculation and Clamping ---
            float output = p_term + integral_term + d_term;
            if (output > 255.0f)
                output = 255.0f;
            if (output < 0.0f)
                output = 0.0f;

            // FIX 3: Actually apply the output to the motor
            set_fan_speed((uint8_t)output);
            ESP_LOGI(TAG, "Setpoint: %.2f°C, Temp: %.2f°C, Fan PWM: %d", setpoint, process_variable, (uint8_t)output);
        }
        else
        {
            // FIX 4: Handle sensor failure gracefully
            ESP_LOGE(TAG, "Failed to read SHTC3. Turning fan off for safety.");
            set_fan_speed(0);  // Set motor to a known, safe state
            is_first_run = true; // Reset state for the next successful read
        }

        // FIX 5: Ensure a consistent loop delay, regardless of success or failure
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}