#include "cJSON.h"
#include "network_manager.h"
#include "sensor_manager.h"
#include "esp_log.h"

#define MIN_SETPOINT_TEMP 18.0f
#define MAX_SETPOINT_TEMP 30.0f
#define SETPOINT_MQTT_TOPIC "device/setpoint/update"
#define POWER_MQTT_TOPIC "device/power"
#define ENVIRONMENT_MQTT_TOPIC "device/environment"
#define MOTION_MQTT_TOPIC "device/motion"

static const char *TAG = "DATA_PUBLISHER";

/**
 * @brief Formats INA226 data as JSON and publishes it to the "device/power" topic.
 */
void publish_power_data(ina226_data_t *data)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Failed to create cJSON object for power data.");
        return;
    }

    cJSON_AddNumberToObject(root, "voltage", data->bus_voltage_v);
    cJSON_AddNumberToObject(root, "current", data->current_a);

    char *json_string = cJSON_PrintUnformatted(root);
    if (json_string)
    {
        network_manager_publish(POWER_MQTT_TOPIC, json_string);
        free(json_string);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to print power data to JSON string.");
    }

    cJSON_Delete(root);
}

/**
 * @brief Formats SHTC3 data as JSON and publishes it to the "device/environment" topic.
 */
void publish_environment_data(shtc3_data_t *data)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Failed to create cJSON object for environment data.");
        return;
    }

    cJSON_AddNumberToObject(root, "temperature", data->temperature_c);
    cJSON_AddNumberToObject(root, "humidity", data->humidity_rh);

    char *json_string = cJSON_PrintUnformatted(root);
    if (json_string)
    {
        network_manager_publish(ENVIRONMENT_MQTT_TOPIC, json_string);
        free(json_string);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to print environment data to JSON string.");
    }

    cJSON_Delete(root);
}

/**
 * @brief Formats MPU6050 data as JSON and publishes it to the "device/motion" topic.
 */
void publish_motion_data(mpu6050_data_t *data)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Failed to create cJSON object for motion data.");
        return;
    }

    // For complex data, it's good practice to use nested objects
    cJSON *accel = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "accelerometer", accel);
    cJSON_AddNumberToObject(accel, "x", data->accel_x_g);
    cJSON_AddNumberToObject(accel, "y", data->accel_y_g);
    cJSON_AddNumberToObject(accel, "z", data->accel_z_g);

    cJSON *gyro = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "gyroscope", gyro);
    cJSON_AddNumberToObject(gyro, "x", data->gyro_x_dps);
    cJSON_AddNumberToObject(gyro, "y", data->gyro_y_dps);
    cJSON_AddNumberToObject(gyro, "z", data->gyro_z_dps);

    cJSON_AddNumberToObject(root, "pitch", data->pitch);
    cJSON_AddNumberToObject(root, "roll", data->roll);

    char *json_string = cJSON_PrintUnformatted(root);
    if (json_string)
    {
        network_manager_publish(MOTION_MQTT_TOPIC, json_string);
        free(json_string);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to print motion data to JSON string.");
    }

    cJSON_Delete(root);
}
void publish_slider_setpoint(uint8_t slider_percentage)
{
    // 1. Map the 0-100% slider value to the temperature range
    float temp_range = MAX_SETPOINT_TEMP - MIN_SETPOINT_TEMP;
    float new_setpoint = MIN_SETPOINT_TEMP + ((float)slider_percentage / 100.0f) * temp_range;

    // 2. Create a cJSON object to hold the data
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Failed to create cJSON object.");
        return;
    }

    // 3. Add both the percentage and the calculated temperature to the object
    cJSON_AddNumberToObject(root, "percentage", slider_percentage);
    cJSON_AddNumberToObject(root, "temperature_c", new_setpoint);

    // 4. Convert the cJSON object to a compact string
    char *json_string = cJSON_PrintUnformatted(root);
    if (json_string)
    {
        // 5. Publish the JSON string using your network manager
        network_manager_publish(SETPOINT_MQTT_TOPIC, json_string);

        // 6. Free the memory used by the string
        free(json_string);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to print cJSON to string.");
    }

    // 7. Free the memory used by the cJSON object
    cJSON_Delete(root);
}