#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include <stdbool.h>

/**
 * @brief Connects to the Wi-Fi network and then to the MQTT broker.
 *
 * This function initializes Wi-Fi, connects to the configured SSID,
 * and upon successful connection, initializes and connects the MQTT client
 * to the specified broker.
 */
void network_manager_init(void);
/**
 * @brief Publishes an MQTT message to a specific topic.
 *
 * @param topic The topic to publish to.
 * @param payload The message payload string.
 * @return true if the message was successfully queued for publishing, false otherwise.
 */

 
bool network_manager_publish(const char *topic, const char *payload);

#endif /* NETWORK_MANAGER_H */
