#pragma once

#include <stdint.h>

typedef enum
{
    MODE_NORMAL = 0,
    MODE_BEARING_WEAR = 1,
    MODE_ROTOR_IMBALANCE = 2,
    MODE_ELECTRICAL = 3,
} collection_mode_t;

void data_collector_start(collection_mode_t mode);
