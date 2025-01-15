#include "recv.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_wifi.h>
#include <nvs_flash.h>

#include <mqtt_client.h>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_mac.h>

#define BROKER_URI "90.89.133.173"
#define TOPIC "sensors/data"
#define PORT 1883
#define USERNAME "wal"
#define PASSWORD "walwal"

#define WIFI_SSID "androyoyo"       // AP SSID
#define WIFI_PASS "cesthonteux"     // AP Password
#define WIFI_CHANNEL 1              // AP Channel
#define MAX_STA_CONN 4              // Maximum simultaneous connections

static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};

static const char *TAG = "wifi_softap";

void wifi_init_softap(void) {
    // Initialize the TCP/IP stack and Wi-Fi event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    // Initialize Wi-Fi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Configure the Wi-Fi access point
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK, // Use WPA2 authentication
        },
    };

    // If no password is set, open the network
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // Set Wi-Fi mode and apply configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi access point initialized. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}



static void mqtt_event_handler(void *event_handler_arg, 
                               esp_event_base_t event_base, 
                               int32_t event_id, 
                               void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        printf("MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, TOPIC, 0);
        printf("Subscribed to topic %s, msg_id=%d", TOPIC, msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        printf("MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        printf("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        printf("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        printf("MQTT_EVENT_DATA");
        printf("Received topic: %.*s\n", event->topic_len, event->topic);
        printf("Received data: %.*s\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        printf("MQTT_EVENT_ERROR");
        break;

    default:
        printf("Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_init()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address = {
                .uri = "",
                .hostname = BROKER_URI,
                .transport = MQTT_TRANSPORT_OVER_TCP,
                .path = TOPIC,
                .port = PORT,
        },
        .credentials = {
            .username = USERNAME,
            .authentication.password = PASSWORD
        }
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    // Publish a test message
    int msg_id = esp_mqtt_client_publish(client, TOPIC, "Hello from ESP32!", 0, 1, 0);
    printf("msg_id: %d", msg_id);
}

void send_mqtt()
{
    printf("Wi-Fi: init...");
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Starting Wi-Fi SoftAP");
    wifi_init_softap();
    printf("MQTT: init...");
    mqtt_init();
}

void recv_task() {
    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);
    lora_receive();  // Set to receive mode

    while (1) {
        if (lora_received()) {
            const int len = lora_receive_packet(_rx_buff, MAX_PACKET_SIZE);
            const int rssi = lora_packet_rssi();
            printf("LoRa: Received packet of size %d\n", len);
            printf("RSSI: %d dB\n", rssi);

            if (len == sizeof(payload_t)) {
                payload_t* received_payload = (payload_t*)_rx_buff;
                printf("Header: 0x%02X\n", received_payload->header);
                printf("Node ID: 0x%04X\n", received_payload->node_id);
                printf("Timestamp: 0x%08lX\n", received_payload->timestamp);
                printf("Battery Level: 0x%02X\n", received_payload->battery_level);
                printf("Temperature: 0x%02X\n", received_payload->temperature);
                printf("Reduced FFT: ");
                for (int i = 0; i < sizeof(received_payload->reduced_fft); i++) {
                    printf("0x%02X ", received_payload->reduced_fft[i]);
                }
                printf("\n");
            } else {
                printf("Received packet size does not match payload size\n");
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}
