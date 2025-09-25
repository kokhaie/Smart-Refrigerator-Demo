#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netif.h"
#include "mosq_broker.h"
#include "mqtt_client.h"
#include <netdb.h>

#include "network_manager.h"

static const char *TAG = "NETWORK_MANAGER";

// --- Configuration ---
#define CONNECTION_TIMEOUT_S 30
#define AP_STATIC_IP "192.168.4.1"
#define AP_STATIC_GATEWAY "192.168.4.1"
#define AP_STATIC_NETMASK "255.255.255.0"

// --- Global Variables ---
static EventGroupHandle_t s_network_event_group;
static bool s_mqtt_connected = false;
static int s_retry_num = 0;

// A pointer to hold the handle of the currently active client (remote or local)
static esp_mqtt_client_handle_t s_active_client = NULL;

// Store the callback function from the main app
static network_status_callback_t s_status_callback = NULL;

/* Event group bits */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define MQTT_CONNECTED_BIT BIT2

// Function to safely call the status callback
static void update_status(network_status_t status)
{
    ESP_LOGI(TAG, "Network status changed to: %d", status);
    if (s_status_callback != NULL)
    {
        s_status_callback(status);
    }
}

static esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler for MQTT events
 *
 * This function is invoked by MQTT client event loop.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        s_mqtt_connected = true;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        if (s_network_event_group != NULL)
        {
            xEventGroupSetBits(s_network_event_group, MQTT_CONNECTED_BIT);
        }
        update_status(NETWORK_STATUS_CONNECTED_INTERNET);

        msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_TOPIC, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        s_mqtt_connected = false;

        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .credentials = {.username = CONFIG_MQTT_USERNAME,
                        .authentication = {.password = CONFIG_MQTT_PASSWORD}},
        .broker = {
            .address.uri = CONFIG_MQTT_BROKER_URI},
        .network = {
            .reconnect_timeout_ms = 10000, // Re-connect after 10 seconds
        },
        .session = {
            .protocol_ver = MQTT_PROTOCOL_V_5,
        },
    };

    esp_mqtt_client_handle_t remote_client = esp_mqtt_client_init(&mqtt_cfg);
    s_active_client = remote_client; // Set the active client to the remote one
    esp_mqtt_client_register_event(s_active_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_active_client);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)

{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        s_mqtt_connected = false;
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_network_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGW(TAG, "Connect to the AP failed");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_network_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        // Log when a client connects to our AP
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}
static void local_broker_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting local Mosquitto MQTT broker task.");
    struct mosq_broker_config config = {
        .host = "0.0.0.0",
        .port = 1883,
        .tls_cfg = NULL};

    // This call will block this task indefinitely, which is what we want.
    mosq_broker_run(&config);

    // This part will likely never be reached unless the broker exits unexpectedly.
    ESP_LOGE(TAG, "Local MQTT broker has stopped!");
    update_status(NETWORK_STATUS_AP_MODE_ACTIVE);

    vTaskDelete(NULL);
}

static void mqtt_app_start_local_client(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.hostname = "127.0.0.1", // Connect to our own IP
        .broker.address.transport = MQTT_TRANSPORT_OVER_TCP,
        .broker.address.port = 1883,
    };

    esp_mqtt_client_handle_t local_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(local_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(local_client);

    // When this local client connects, the mqtt_event_handler will set s_mqtt_connected = true
    // and we will set our active client handle.
    // For simplicity in this example, we'll assign it here. A more robust solution might
    // wait for the MQTT_EVENT_CONNECTED for the local client.
    s_active_client = local_client;
}

static void wifi_init_ap(void)
{
    update_status(NETWORK_STATUS_STARTING_AP_MODE);

    // Stop the station mode and de-init networking stack
    // ESP_ERROR_CHECK(esp_wifi_stop());
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif)
    {
        esp_netif_destroy(sta_netif);
    }

    // Create AP network interface
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

    // Configure static IP for the AP
    esp_netif_dhcps_stop(ap_netif); // Stop DHCP server to set static IP
    esp_netif_ip_info_t ip_info;
    inet_pton(AF_INET, AP_STATIC_IP, &ip_info.ip);
    inet_pton(AF_INET, AP_STATIC_GATEWAY, &ip_info.gw);
    inet_pton(AF_INET, AP_STATIC_NETMASK, &ip_info.netmask);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif)); // Restart DHCP server

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESPWIWI", // You need to set this in menuconfig
            .ssid_len = strlen("ESPWIWI"),
            .password = "ESPWIWII", // And this one
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen("ESPWIWI") == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_ap finished. SSID:%s password:%s",
             "ESPWIWI", "ESPWIWI");

    // Start the local broker, THEN connect to it as a client
    xTaskCreatePinnedToCore(
        local_broker_task, // Function to implement the task
        "mqtt_broker",     // Name of the task
        4096,              // Stack size in words
        NULL,              // Task input parameter
        9,                 // Priority of the task
        NULL, 0            // Task handle
    );

    vTaskDelay(pdMS_TO_TICKS(500));
    mqtt_app_start_local_client();
}

bool network_manager_publish(const char *topic, const char *payload)
{
    if (!s_mqtt_connected || s_active_client == NULL)
    {
        ESP_LOGE(TAG, "Cannot publish, MQTT client is not connected.");
        return false;
    }

    int msg_id = esp_mqtt_client_publish(s_active_client, topic, payload, 0, 1, 0);

    if (msg_id == -1)
    {
        ESP_LOGE(TAG, "Failed to publish message to topic %s", topic);
        return false;
    }

    ESP_LOGI(TAG, "Successfully published to topic %s, msg_id=%d", topic, msg_id);
    return true;
}

static void wifi_configure_sta(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_LOGI(TAG, "Wi-Fi configured for STA mode.");
}

static void network_manager_task(void *pvParameters)
{
    // --- 1. ONE-TIME INITIALIZATION ---
    s_network_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default STA interface
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif); // Ensure it was created

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers ONCE
    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_ip_event));

    // --- 2. ATTEMPT TO CONNECT IN STA MODE ---
    update_status(NETWORK_STATUS_CONNECTING_WIFI);
    wifi_configure_sta();
    ESP_ERROR_CHECK(esp_wifi_start());
    mqtt_app_start(); // Start the remote MQTT client

    ESP_LOGI(TAG, "Waiting for connection to Wi-Fi and MQTT Broker (%ds timeout)...", CONNECTION_TIMEOUT_S);
    EventBits_t bits = xEventGroupWaitBits(s_network_event_group,
                                           MQTT_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, // Don't clear bits on exit
                                           pdFALSE, // Wait for EITHER bit
                                           pdMS_TO_TICKS(CONNECTION_TIMEOUT_S * 1000));

    // --- 3. CHECK RESULT AND DECIDE ACTION ---
    bool connected_successfully = (bits & MQTT_CONNECTED_BIT) != 0;

    if (connected_successfully)
    {
        ESP_LOGI(TAG, "Successfully connected to Wi-Fi and MQTT Broker.");
        // Unregister handlers we no longer need if we are stable
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_wifi_event));
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_ip_event));
    }
    else
    {
        // --- 4. FAILED - CLEAN UP AND SWITCH TO AP MODE ---
        update_status(NETWORK_STATUS_CONNECTION_FAILED);
        if ((bits & WIFI_FAIL_BIT) != 0)
        {
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi after maximum retries.");
        }
        else
        {
            ESP_LOGE(TAG, "Timed out waiting for a connection.");
        }
        ESP_LOGI(TAG, "Switching to AP mode...");

        // 1. Unregister event handlers
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_wifi_event));
        ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_ip_event));

        // 2. Stop and destroy MQTT client
        if (s_active_client)
        {
            esp_mqtt_client_stop(s_active_client);
            esp_mqtt_client_destroy(s_active_client);
            s_active_client = NULL;
        }

        // 3. Stop Wi-Fi and destroy STA interface
        esp_wifi_stop();
        esp_netif_destroy(sta_netif);

        // 4. Start AP mode
        wifi_init_ap();
    }

    // --- 5. CLEAN UP TASK RESOURCES ---
    vEventGroupDelete(s_network_event_group);
    s_network_event_group = NULL;
    vTaskDelete(NULL);
}
void network_manager_start(network_status_callback_t status_callback)
{
    s_status_callback = status_callback; // Save the callback
    update_status(NETWORK_STATUS_INITIALIZING);
    xTaskCreatePinnedToCore(network_manager_task, "network_manager", 8192, NULL, 10, NULL, 0);
}