#include "anomaly_detector.h"
#include "sensor_manager.h"
#include "model/rf_model_v2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const char *TAG = "ANOMALY_DETECTOR";

// Sampling configuration
#define SAMPLE_RATE_HZ 1000.0f
#define WINDOW_SIZE BATCH_SIZE
#define SAMPLE_TIMEOUT_MS 200

// Task configuration
#define TASK_STACK_SIZE 8192
#define TASK_PRIORITY 8
#define TASK_CORE_ID tskNO_AFFINITY

// Helper for probability clamp
#define PROB_CLAMP(x)         ((x) < 0.0f ? 0.0f : ((x) > 1.0f ? 1.0f : (x)))

void compute_frequency_features(const float *data,
                                       int count,
                                       float sample_rate,
                                       float *scratch_power,
                                       float *out_dominant_freq,
                                       float *out_entropy);

void compute_window_features(const float *magnitudes,
                                    int count,
                                    float pwm_mean,
                                    float *scratch_power,
                                    float out_features[ANOMALY_FEATURE_COUNT]);

void anomaly_detector_task(void *arg);

static SemaphoreHandle_t s_result_mutex = NULL;
static anomaly_detector_result_t s_latest_result = {0};
static bool s_result_ready = false;
static TaskHandle_t s_task_handle = NULL;
static anomaly_detector_callback_t s_callback = NULL;
static void *s_callback_ctx = NULL;
static volatile uint8_t s_current_pwm = 0;
static float s_anomaly_threshold = 0.0f;

static anomaly_detector_class_t map_model_class(int model_class)
{
    switch (model_class)
    {
    case 0:
        return ANOMALY_CLASS_NORMAL;
    case 1:
        return ANOMALY_CLASS_BEARING_WEAR;
    case 2:
        return ANOMALY_CLASS_IMBALANCE;
    case 3:
        return ANOMALY_CLASS_ELECTRICAL;
    default:
        return ANOMALY_CLASS_EXTERNAL_EVENT;
    }
}

/**
 * @brief Computes frequency-domain features (dominant frequency, spectral entropy) from time-series data.
 *
 * NOTE: This implementation matches the Python reference:
 * 1.  Dominant Frequency: Calculated from the max magnitude of bins 1 to N/2 (ignores DC component).
 * 2.  Spectral Entropy: Calculated using all bins 0 to N/2 (includes DC component).
 */
void compute_frequency_features(const float *data,
                                int count,
                                float sample_rate,
                                float *scratch_magnitude,
                                float *out_dominant_freq,
                                float *out_entropy)
{
    if (out_dominant_freq)
    {
        *out_dominant_freq = 0.0f;
    }
    if (out_entropy)
    {
        *out_entropy = 0.0f;
    }

    if (!data || !scratch_magnitude || !out_dominant_freq || !out_entropy || count <= 1 || sample_rate <= 0.0f)
    {
        return;
    }

    int half_bins = count / 2;
    if (half_bins <= 1)
    {
        return;
    }

    memset(scratch_magnitude, 0, sizeof(float) * (size_t)half_bins);

    // --- MODIFIED: Calculate DC component (bin 0) first ---
    // This is required for the spectral entropy calculation to match Python's np.sum(fft_mag)
    float dc_real = 0.0f;
    for (int n = 0; n < count; ++n)
    {
        dc_real += data[n];
    }
    float dc_magnitude = fabsf(dc_real);
    scratch_magnitude[0] = dc_magnitude;
    float total_magnitude = dc_magnitude;
    // --- END MODIFICATION ---

    float max_magnitude = 0.0f; // Max magnitude for *dominant frequency* (skips bin 0)
    int max_bin = 0;            // Index for dominant frequency
    const float inv_count = 1.0f / (float)count;

    // Loop from k = 1 to compute AC components
    for (int k = 1; k < half_bins; ++k)
    {
        float angle_step = -2.0f * (float)M_PI * (float)k * inv_count;
        float cos_step = cosf(angle_step);
        float sin_step = sinf(angle_step);
        float cos_curr = 1.0f;
        float sin_curr = 0.0f;
        float real = 0.0f;
        float imag = 0.0f;

        for (int n = 0; n < count; ++n)
        {
            float value = data[n];
            real += value * cos_curr;
            imag += value * sin_curr;

            float next_cos = (cos_curr * cos_step) - (sin_curr * sin_step);
            float next_sin = (sin_curr * cos_step) + (cos_curr * sin_step);

            cos_curr = next_cos;
            sin_curr = next_sin;

            // Re-normalize to prevent floating point drift in long loops
            if ((n & 0x7F) == 0)
            {
                float mag_norm = sqrtf((cos_curr * cos_curr) + (sin_curr * sin_curr));
                if (mag_norm > 0.0f)
                {
                    cos_curr /= mag_norm;
                    sin_curr /= mag_norm;
                }
            }
        }

        float magnitude = sqrtf((real * real) + (imag * imag));
        scratch_magnitude[k] = magnitude;
        total_magnitude += magnitude; // Add to total for entropy

        // Find dominant frequency (skipping bin 0, matches Python's fft_mag[1:])
        if (magnitude > max_magnitude)
        {
            max_magnitude = magnitude;
            max_bin = k;
        }

        if ((k & 0x0F) == 0)
        {
            taskYIELD();
        }
    }

    // Use total_magnitude for entropy check
    if (total_magnitude <= 1e-12f)
    {
        return; // Dominant freq and entropy will be 0
    }

    // Dominant freq calculation is unchanged (based on max_bin from k=1)
    *out_dominant_freq = ((float)max_bin * sample_rate) / (float)count;

    float entropy = 0.0f;
    float inv_total_magnitude = 1.0f / total_magnitude;

    // --- MODIFIED: Start entropy loop from k = 0 ---
    // This includes the DC component in the entropy calculation
    for (int k = 0; k < half_bins; ++k)
    {
        float magnitude = scratch_magnitude[k];
        if (magnitude <= 1e-12f) // Use epsilon check
        {
            continue;
        }

        float probability = magnitude * inv_total_magnitude;
        // Add epsilon inside logf to match Python's np.log(fft_mag_norm + 1e-10)
        entropy -= probability * logf(probability + 1e-12f);
    }
    // --- END MODIFICATION ---

    *out_entropy = entropy;
}


void compute_window_features(const float *magnitudes,
                             int count,
                             float pwm_mean,
                             float *scratch_magnitude,
                             float out_features[ANOMALY_FEATURE_COUNT])
{
    if (!out_features)
    {
        return;
    }

    for (int i = 0; i < ANOMALY_FEATURE_COUNT; ++i)
    {
        out_features[i] = 0.0f;
    }

    if (!magnitudes || count <= 0)
    {
        out_features[3] = pwm_mean;
        return;
    }

    float sum = 0.0f;
    float sum_sq = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        float value = magnitudes[i];
        sum += value;
        sum_sq += value * value;
    }

    float mean = sum / (float)count;
    // RMS: This matches np.sqrt(np.mean(accel**2))
    float rms = sqrtf(sum_sq / (float)count);

    float accum2 = 0.0f;
    float accum4 = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        float diff = magnitudes[i] - mean;
        float diff_sq = diff * diff;
        accum2 += diff_sq;
        accum4 += diff_sq * diff_sq;
    }

    float kurtosis = 0.0f;
    if (accum2 > 1e-12f)
    {
        float variance = accum2 / (float)count;
        float fourth_moment = accum4 / (float)count;
        // Excess Kurtosis: This matches scipy.stats.kurtosis(accel)
        kurtosis = (fourth_moment / (variance * variance)) - 3.0f; 
    }

    float dominant_freq = 0.0f;
    float spectral_entropy = 0.0f;
    compute_frequency_features(magnitudes,
                               count,
                               SAMPLE_RATE_HZ,
                               scratch_magnitude,
                               &dominant_freq,
                               &spectral_entropy);

    if (pwm_mean < 0.0f)
    {
        pwm_mean = 0.0f;
    }
    else if (pwm_mean > 100.0f)
    {
        pwm_mean = 100.0f;
    }

    out_features[0] = rms;
    out_features[1] = kurtosis;
    out_features[2] = dominant_freq;
    out_features[3] = pwm_mean;
    out_features[4] = spectral_entropy;
}

esp_err_t anomaly_detector_init(anomaly_detector_callback_t callback, void *user_ctx, float anomaly_threshold)
{
    if (s_task_handle != NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    if (!s_result_mutex)
    {
        s_result_mutex = xSemaphoreCreateMutex();
        if (!s_result_mutex)
        {
            return ESP_ERR_NO_MEM;
        }
    }

    s_callback = callback;
    s_callback_ctx = user_ctx;
    s_result_ready = false;
    s_anomaly_threshold = anomaly_threshold;

    BaseType_t rc = xTaskCreatePinnedToCore(
        anomaly_detector_task,
        "anomaly_detector",
        TASK_STACK_SIZE,
        NULL,
        TASK_PRIORITY,
        &s_task_handle,
        TASK_CORE_ID);

    if (rc != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create anomaly detector task");
        s_task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

bool anomaly_detector_get_latest(anomaly_detector_result_t *out)
{
    if (!out || !s_result_mutex)
    {
        return false;
    }

    bool success = false;
    if (xSemaphoreTake(s_result_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (s_result_ready)
        {
            *out = s_latest_result;
            success = true;
        }
        xSemaphoreGive(s_result_mutex);
    }

    return success;
}

void anomaly_detector_record_pwm(uint8_t pwm_percent)
{
    if (pwm_percent > 100)
    {
        pwm_percent = 100;
    }
    s_current_pwm = pwm_percent;
}

void anomaly_detector_task(void *arg)
{
    (void)arg;

    float *magnitude_buffer = (float *)malloc(sizeof(float) * WINDOW_SIZE);
    if (!magnitude_buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate magnitude buffer");
        vTaskDelete(NULL);
        return;
    }

    // Allocate buffer for DFT magnitudes (0 to N/2)
    float *power_buffer = (float *)malloc(sizeof(float) * (WINDOW_SIZE / 2));
    if (!power_buffer)
    {
        ESP_LOGE(TAG, "Failed to allocate power buffer");
        free(magnitude_buffer);
        vTaskDelete(NULL);
        return;
    }

    memset(magnitude_buffer, 0, sizeof(float) * WINDOW_SIZE);
    memset(power_buffer, 0, sizeof(float) * (WINDOW_SIZE / 2));

    ESP_LOGI(TAG, "Anomaly detector task started (window=%d samples)", WINDOW_SIZE);

    for (;;)
    {
        int collected = 0;
        double pwm_sum = 0.0;
        uint64_t window_start_us = 0;
        uint64_t window_end_us = 0;
        int timeout_streak = 0;

        while (collected < WINDOW_SIZE)
        {
            synchronized_sample_t sample;
            if (!sensor_manager_get_raw_sample(&sample, pdMS_TO_TICKS(SAMPLE_TIMEOUT_MS)))
            {
                timeout_streak++;
                if (timeout_streak == 1 || (timeout_streak % 50) == 0)
                {
                    ESP_LOGW(TAG, "Timeout waiting for raw sample (%d/%d)", collected, WINDOW_SIZE);
                }
                continue;
            }

            timeout_streak = 0;
            if (collected == 0)
            {
                window_start_us = sample.timestamp_us;
            }

            window_end_us = sample.timestamp_us;
            magnitude_buffer[collected] = sample.magnitude;
            pwm_sum += (double)s_current_pwm;
            collected++;
        }

        if (collected <= 0)
        {
            continue;
        }

        float pwm_mean = (float)(pwm_sum / (double)collected);

        float features[ANOMALY_FEATURE_COUNT];
        compute_window_features(magnitude_buffer,
                                collected,
                                pwm_mean,
                                power_buffer, // Used as scratch_magnitude
                                features);

        float probabilities[RF_NUM_CLASSES];
        rf_predict(features, probabilities);

        for (int i = 0; i < RF_NUM_CLASSES; ++i)
        {
            probabilities[i] = PROB_CLAMP(probabilities[i]);
        }

        int model_class = 0;
        float best_prob = probabilities[0];
        for (int i = 1; i < RF_NUM_CLASSES; ++i)
        {
            if (probabilities[i] > best_prob)
            {
                best_prob = probabilities[i];
                model_class = i;
            }
        }

        anomaly_detector_result_t result;
        memset(&result, 0, sizeof(result));

        result.classification = map_model_class(model_class);
        result.model_class = (uint8_t)model_class;
        result.anomaly_threshold = s_anomaly_threshold;

        // Determine if anomaly based on threshold
        bool is_anomaly = false;
        if (result.classification != ANOMALY_CLASS_NORMAL)
        {
            if (best_prob >= s_anomaly_threshold)
            {
                is_anomaly = true;
            }
        }
        result.is_anomaly = is_anomaly;

        result.probability_normal = probabilities[0];
        result.probability_bearing_wear = (RF_NUM_CLASSES > 1) ? probabilities[1] : 0.0f;
        result.probability_imbalance = (RF_NUM_CLASSES > 2) ? probabilities[2] : 0.0f;
        result.probability_electrical = (RF_NUM_CLASSES > 3) ? probabilities[3] : 0.0f;

        for (int i = 0; i < RF_NUM_CLASSES; ++i)
        {
            result.class_probabilities[i] = probabilities[i];
        }

        for (int i = 0; i < ANOMALY_FEATURE_COUNT; ++i)
        {
            result.features[i] = features[i];
        }

        result.window_start_us = window_start_us;
        result.window_end_us = window_end_us;

        if (s_result_mutex && xSemaphoreTake(s_result_mutex, portMAX_DELAY) == pdTRUE)
        {
            s_latest_result = result;
            s_result_ready = true;
            xSemaphoreGive(s_result_mutex);
        }

        if (s_callback)
        {
            s_callback(&result, s_callback_ctx);
        }

        ESP_LOGI(TAG,
                 "Inference: class=%d Pn=%.3f Pb=%.3f Pi=%.3f Pe=%.3f rms=%.5f kurt=%.3f freq=%.1fHz pwm=%.1f entropy=%.3f",
                 model_class,
                 result.probability_normal,
                 result.probability_bearing_wear,
                 result.probability_imbalance,
                 result.probability_electrical,
                 result.features[0],
                 result.features[1],
                 result.features[2],
                 result.features[3],
                 result.features[4]);
    }
}
