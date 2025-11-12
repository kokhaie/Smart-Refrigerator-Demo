#pragma once

#include <stdint.h>

typedef enum
{
    FAULT_TYPE_NORMAL = 0,
    FAULT_TYPE_BEARING_WEAR = 1,
    FAULT_TYPE_IMBALANCE = 2,
    FAULT_TYPE_ELECTRICAL = 3,
} fault_type_t;

void fault_simulator_run(fault_type_t fault, uint32_t duration_ms);
