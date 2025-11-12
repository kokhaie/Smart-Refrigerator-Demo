#pragma once

#include <stdint.h>

void vibration_pattern_bearing_wear(float amplitude_0_to_1, int duration_ms);
void vibration_pattern_imbalance(uint8_t fan_speed, float amplitude_0_to_1, int duration_ms);
void vibration_pattern_electrical(float amplitude_0_to_1, int duration_ms);
void apply_vibration_speed(uint8_t speed_percent);

