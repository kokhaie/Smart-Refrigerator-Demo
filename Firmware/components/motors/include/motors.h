#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

/**
 * @brief Initializes the LEDC peripheral for motor control.
 */
void motors_init(void);

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

/**
 * @brief Sets the fan speed with a kick-start for low speeds.
 *
 * @param percentage The desired speed from 0 to 100.
 */
#endif // MOTORS_H