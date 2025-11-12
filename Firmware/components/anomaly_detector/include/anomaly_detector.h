#ifndef ANOMALY_DETECTOR_H
#define ANOMALY_DETECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ANOMALY_FEATURE_COUNT 5
#define ANOMALY_MODEL_CLASS_COUNT 4

typedef enum
{
    ANOMALY_CLASS_NORMAL = 0,
    ANOMALY_CLASS_BEARING_WEAR = 1,
    ANOMALY_CLASS_IMBALANCE = 2,
    ANOMALY_CLASS_ELECTRICAL = 3,
    ANOMALY_CLASS_EXTERNAL_EVENT = 4
} anomaly_detector_class_t;

typedef struct
{
    anomaly_detector_class_t classification;
    float probability_normal;
    float probability_bearing_wear;
    float probability_imbalance;
    float probability_electrical;
    bool is_anomaly;
    uint8_t model_class;
    float class_probabilities[ANOMALY_MODEL_CLASS_COUNT];
    uint64_t window_start_us;
    uint64_t window_end_us;
    float features[ANOMALY_FEATURE_COUNT];
    float anomaly_threshold;
} anomaly_detector_result_t;

typedef void (*anomaly_detector_callback_t)(const anomaly_detector_result_t *result, void *user_ctx);

esp_err_t anomaly_detector_init(anomaly_detector_callback_t callback, void *user_ctx, float anomaly_threshold);
bool anomaly_detector_get_latest(anomaly_detector_result_t *out);
void anomaly_detector_record_pwm(uint8_t pwm_percent);

#ifdef __cplusplus
}
#endif

#endif /* ANOMALY_DETECTOR_H */
