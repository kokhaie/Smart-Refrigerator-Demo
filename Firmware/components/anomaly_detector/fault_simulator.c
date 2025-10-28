#include "fault_simulator.h"
#include "vibration_patterns.h"
#include "motor_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void fault_simulator_run(fault_type_t fault, uint32_t duration_ms)
{
    switch (fault)
    {
    case FAULT_TYPE_NORMAL:
        apply_vibration_speed(0);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        break;
    case FAULT_TYPE_BEARING_WEAR:
        vibration_pattern_bearing_wear(0.6f, duration_ms);
        break;
    case FAULT_TYPE_IMBALANCE:
        // Imbalance is speed dependent, so we need to set a fan speed
        set_fan_speed(80);
        vibration_pattern_imbalance(80, 0.7f, duration_ms);
        break;
    case FAULT_TYPE_ELECTRICAL:
        vibration_pattern_electrical(0.9f, duration_ms);
        break;
    }
    // Turn off vibration after simulation
    apply_vibration_speed(0);
}
