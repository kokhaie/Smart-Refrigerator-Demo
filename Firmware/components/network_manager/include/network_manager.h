// network_manager.h

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <stdbool.h>

// Enum to represent the network status for your LCD
typedef enum {
    NETWORK_STATUS_INITIALIZING,
    NETWORK_STATUS_CONNECTING_WIFI,
    NETWORK_STATUS_CONNECTING_MQTT,
    NETWORK_STATUS_CONNECTED_INTERNET,
    NETWORK_STATUS_CONNECTION_FAILED,
    NETWORK_STATUS_STARTING_AP_MODE,
    NETWORK_STATUS_STARTING_LOCAL_BROKER,
    NETWORK_STATUS_AP_MODE_ACTIVE,
} network_status_t;

// Define the callback function pointer type
typedef void (*network_status_callback_t)(network_status_t status);

/**
 * @brief Starts the network manager.
 * * It will first attempt to connect to the configured Wi-Fi and MQTT broker.
 * If it fails after a timeout, it will switch to AP mode and start a local MQTT broker.
 * * @param status_callback A function pointer that will be called with status updates.
 */
void network_manager_start(network_status_callback_t status_callback);

/**
 * @brief Publish a message to the MQTT broker (either remote or local).
 * * @param topic The MQTT topic to publish to.
 * @param payload The message payload.
 * @return true if publishing was successful, false otherwise.
 */
bool network_manager_publish(const char *topic, const char *payload);

#endif // NETWORK_MANAGER_H