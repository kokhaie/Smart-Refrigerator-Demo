#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connects to the Wi-Fi network and then to the MQTT broker.
 *
 * This function initializes Wi-Fi, connects to the configured SSID,
 * and upon successful connection, initializes and connects the MQTT client
 * to the specified broker.
 */
void network_manager_init(void);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_MANAGER_H */
