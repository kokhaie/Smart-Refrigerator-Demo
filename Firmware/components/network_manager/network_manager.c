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

// --- MQTT Publish Queue ---
#define MQTT_PUBLISH_QUEUE_LEN 20
typedef struct
{
    char topic[64];
    char payload[256];
} mqtt_publish_msg_t;

static QueueHandle_t s_publish_queue = NULL;

// Function to safely call the status callback
static void update_status(network_status_t status)
{
    ESP_LOGI(TAG, "Network status changed to: %d", status);
    if (s_status_callback != NULL)
    {
        s_status_callback(status);
    }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler for MQTT events
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
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

        esp_mqtt_client_subscribe(event->client, CONFIG_MQTT_TOPIC, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        s_mqtt_connected = false;
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
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
        .broker = {.address.uri = CONFIG_MQTT_BROKER_URI},
        .network = {.reconnect_timeout_ms = 10000}, // 10s reconnect
        .session = {.protocol_ver = MQTT_PROTOCOL_V_5},
    };

    esp_mqtt_client_handle_t remote_client = esp_mqtt_client_init(&mqtt_cfg);
    s_active_client = remote_client;
    esp_mqtt_client_register_event(s_active_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_active_client);
}

// --- Publisher Worker Task ---
static void mqtt_publisher_task(void *arg)
{
    mqtt_publish_msg_t msg;
    for (;;)
    {
        if (xQueueReceive(s_publish_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            if (s_mqtt_connected && s_active_client != NULL)
            {
                int msg_id = esp_mqtt_client_publish(
                    s_active_client,
                    msg.topic,
                    msg.payload,
                    0, // use strlen internally
                    1, // QoS 1
                    0  // no retain
                );

                if (msg_id == -1)
                {
                    ESP_LOGW(TAG, "Failed to publish to topic %s", msg.topic);
                }
                else
                {
                    ESP_LOGI(TAG, "Published to %s, msg_id=%d", msg.topic, msg_id);
                }
            }
            else
            {
                ESP_LOGW(TAG, "MQTT not connected, dropping message on %s", msg.topic);
            }
        }
    }
}

// --- Public API for publishing ---
bool network_manager_publish(const char *topic, const char *payload)
{
    if (!s_publish_queue)
    {
        ESP_LOGE(TAG, "Publish queue not initialized");
        return false;
    }

    mqtt_publish_msg_t msg;
    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = '\0';
    strncpy(msg.payload, payload, sizeof(msg.payload) - 1);
    msg.payload[sizeof(msg.payload) - 1] = '\0';

    if (xQueueSend(s_publish_queue, &msg, 0) != pdPASS)
    {
        ESP_LOGW(TAG, "Publish queue full, dropping message for topic %s", topic);
        return false;
    }
    return true;
}

// --- Wi-Fi Handlers ---
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
}

// --- Main Task ---
static void network_manager_task(void *pvParameters)
{
    s_network_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_ip_event));

    update_status(NETWORK_STATUS_CONNECTING_WIFI);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    mqtt_app_start();

    ESP_LOGI(TAG, "Waiting for connection to Wi-Fi and MQTT Broker (%ds timeout)...", CONNECTION_TIMEOUT_S);
    EventBits_t bits = xEventGroupWaitBits(
        s_network_event_group,
        MQTT_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(CONNECTION_TIMEOUT_S * 1000));

    bool connected_successfully = (bits & MQTT_CONNECTED_BIT) != 0;
    if (connected_successfully)
    {
        ESP_LOGI(TAG, "Successfully connected to Wi-Fi and MQTT Broker.");
    }
    else
    {
        update_status(NETWORK_STATUS_CONNECTION_FAILED);
        ESP_LOGE(TAG, "Failed to connect.");
    }

    // --- Start Publisher Task ---
    s_publish_queue = xQueueCreate(MQTT_PUBLISH_QUEUE_LEN, sizeof(mqtt_publish_msg_t));
    if (s_publish_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create publish queue!");
    }
    else
    {
        xTaskCreatePinnedToCore(mqtt_publisher_task, "mqtt_publisher", 4096, NULL, 8, NULL, 1);
    }

    vTaskDelete(NULL);
}

void network_manager_start(network_status_callback_t status_callback)
{
    s_status_callback = status_callback;
    update_status(NETWORK_STATUS_INITIALIZING);
    xTaskCreatePinnedToCore(network_manager_task, "network_manager", 8192, NULL, 10, NULL, 0);
}
