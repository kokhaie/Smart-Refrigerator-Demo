#ifndef BUSINESS_LOGIC_H
#define BUSINESS_LOGIC_H

#include "freertos/queue.h"

extern QueueHandle_t g_setpoint_queue;
void update_setpoint(float setpoint);
void pid_fan_control_task(void *pvParameters);

#endif // BUSINESS_LOGIC_H