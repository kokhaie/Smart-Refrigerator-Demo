#ifndef BUSINESS_LOGIC_H
#define BUSINESS_LOGIC_H

#include <stdbool.h>
#include "freertos/queue.h"

typedef enum
{
    BUSINESS_LOGIC_MODE_SMART = 0,
    BUSINESS_LOGIC_MODE_ECO,
    BUSINESS_LOGIC_MODE_RAPID,
} business_logic_mode_t;

typedef struct
{
    business_logic_mode_t mode;
    float duty_scale;          // Multiplier applied to PID duty output (1.0 = unchanged)
    float max_duty_percent;    // Hard ceiling applied after scaling (<=100)
    float ramp_up_rate;        // %/cycle when ramping up
    float ramp_down_rate;      // %/cycle when ramping down
    float revert_tolerance_c;  // Temperature tolerance for considering the target reached
} business_logic_mode_profile_t;

typedef void (*business_logic_temp_observer_t)(float current_temp_c);
typedef void (*business_logic_mode_reached_cb_t)(business_logic_mode_t mode);

extern QueueHandle_t g_setpoint_queue;

void update_setpoint(float setpoint);
void pid_fan_control_task(void *pvParameters);
void business_logic_start(void);

void business_logic_apply_mode_profile(const business_logic_mode_profile_t *profile);
void business_logic_register_temperature_observer(business_logic_temp_observer_t observer);
void business_logic_register_mode_reached_callback(business_logic_mode_reached_cb_t cb);

#endif // BUSINESS_LOGIC_H
