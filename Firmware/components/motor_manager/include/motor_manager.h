#ifndef MOTOR_MANAGER_H
#define MOTOR_MANAGER_H

#include <stdint.h>

/**
 * @brief Initializes the LEDC peripheral for motor control.
 */
void motor_manager_init(void);

/**
 * @brief Sets the fan speed.
 *
 * @param percentage The desired speed from 0 to 100.
 */
void set_fan_speed(uint8_t percentage);

/**
 * @brief Sets the vibration motor speed.
 *
 * @param percentage The desired speed from 0 to 100.
 */
void set_vibration_speed(uint8_t percentage);

#endif // MOTOR_MANAGER_H
