#include "cJSON.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MIN_SETPOINT_TEMP 18.0f
#define MAX_SETPOINT_TEMP 30.0f
#define SETPOINT_MQTT_TOPIC "device/setpoint/update"

// Training data topic (batches for AI)
#define TRAINING_TOPIC "device/training/samples"
// Real-time visualization topic
#define STREAM_TOPIC "device/realtime/samples"

#define PUBLISH_MODE_BATCH 1
#define PUBLISH_MODE_STREAM 2

static int publish_mode = PUBLISH_MODE_STREAM;
static const char *TAG = "DATA_PUBLISHER";

// ------------------------- JSON Publishers -------------------------

void publish_slider_setpoint(uint8_t slider_percentage)
{
    float temp_range = MAX_SETPOINT_TEMP - MIN_SETPOINT_TEMP;
    float new_setpoint = MIN_SETPOINT_TEMP + ((float)slider_percentage / 100.0f) * temp_range;

    cJSON *root = cJSON_CreateObject();
    if (!root)
        return;

    cJSON_AddNumberToObject(root, "percentage", slider_percentage);
    cJSON_AddNumberToObject(root, "temperature_c", new_setpoint);

    char *json_string = cJSON_PrintUnformatted(root);
    if (json_string)
    {
        // Push to TX queue (non-blocking)
        network_manager_publish(SETPOINT_MQTT_TOPIC, json_string);
        free(json_string);
    }
    cJSON_Delete(root);
}

// ------------------------- Publisher Task -------------------------

static void publisher_task(void *arg)
{
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t stream_interval = pdMS_TO_TICKS(100); // ~10 Hz

    for (;;)
    {
        if (publish_mode == PUBLISH_MODE_STREAM)
        {
            // Stream mode: downsampled real-time samples
            synchronized_sample_t pkt;
            if (sensor_manager_get_next_sample(&pkt, portMAX_DELAY))
            {
                vTaskDelayUntil(&last_wake, stream_interval); // precise 10 Hz pacing

                char json[128];
                snprintf(json, sizeof(json),
                         "{\"t\":%lld,\"ax\":%.3f,\"ay\":%.3f,\"az\":%.3f,"
                         "\"I\":%.3f,\"T\":%.2f}",
                         pkt.timestamp_us,
                         pkt.accel_x_g, pkt.accel_y_g, pkt.accel_z_g,
                         pkt.latest_current_a,
                         pkt.latest_temperature_c);

                // Non-blocking publish (queued)
                network_manager_publish(STREAM_TOPIC, json);
            }
        }
        else if (publish_mode == PUBLISH_MODE_BATCH)
        {
            // Batch mode: fetch 1000 samples and send them as JSON array
            synchronized_sample_t buf[BATCH_SIZE];
            int count = 0;
            if (sensor_manager_get_batch(buf, &count, portMAX_DELAY))
            {
                cJSON *root = cJSON_CreateArray();
                for (int i = 0; i < count; i++)
                {
                    cJSON *s = cJSON_CreateObject();
                    cJSON_AddNumberToObject(s, "t", buf[i].timestamp_us);
                    cJSON_AddNumberToObject(s, "ax", buf[i].accel_x_g);
                    cJSON_AddNumberToObject(s, "ay", buf[i].accel_y_g);
                    cJSON_AddNumberToObject(s, "az", buf[i].accel_z_g);
                    cJSON_AddNumberToObject(s, "I", buf[i].latest_current_a);
                    cJSON_AddNumberToObject(s, "T", buf[i].latest_temperature_c);
                    cJSON_AddItemToArray(root, s);
                }

                char *json_string = cJSON_PrintUnformatted(root);
                if (json_string)
                {
                    // Non-blocking publish (queued)
                    network_manager_publish(TRAINING_TOPIC, json_string);
                    free(json_string);
                }
                cJSON_Delete(root);
            }
        }
    }
}

void data_publisher_start(void)
{
    xTaskCreatePinnedToCore(publisher_task, "publisher",
                            8192, NULL, 5, NULL, 1);
}
