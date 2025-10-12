#ifndef DATA_PUBLISHER_H
#define DATA_PUBLISHER_H

#include "sensor_manager.h"

void publish_slider_setpoint(uint8_t slider_percentage);
void publish_training_batch(synchronized_sample_t *batch, int count);
void data_publisher_start(void);


#endif // DATA_PUBLISHER_H