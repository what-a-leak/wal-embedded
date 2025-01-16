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

#define WIFI_SSID "androyoyo"   // AP SSID
#define WIFI_PASS "cesthonteux" // AP Password

static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};

static const char *TAG = "wifi_connect";
esp_ip4_addr_t static_addr;

// Event handler for Wi-Fi events
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi disconnected, reconnecting...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        static_addr = event->ip_info.ip;
        ESP_LOGI(TAG, "Connected! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to SSID: %s...", WIFI_SSID);
}

#define BROKER_URI "mqtt://wal:walwal@90.89.133.173:1883"
#define TOPIC "sensors/data"

static void mqtt_event_handler(void *event_handler_arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id)
    {
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
        .broker.address.uri = BROKER_URI,
    };
    printf("Created config\n");

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    printf("MQTT Client init\n");
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    printf("MQTT Registered event\n");
    esp_mqtt_client_start(client);
    printf("MQTT started client\n");

    // Publish a test message
    int msg_id = esp_mqtt_client_publish(client, TOPIC, "Hello from ESP32!", 0, 1, 0);
    printf("msg_id: %d", msg_id);
}

void send_mqtt()
{
    printf("Starting Wi-Fi Init...\n");
    wifi_init_sta();
    printf("Wi-Fi OK!\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("MQTT init...\n");
    mqtt_init();
    while (1)
    {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void recv_task()
{
    lora_init();
    lora_set_frequency(433e6);
    lora_enable_crc();
    lora_set_coding_rate(CODING_RATE);
    lora_set_bandwidth(BANDWIDTH);
    lora_set_spreading_factor(SPREADING_FACTOR);
    lora_receive(); // Set to receive mode

    while (1)
    {
        if (lora_received())
        {
            const int len = lora_receive_packet(_rx_buff, MAX_PACKET_SIZE);
            const int rssi = lora_packet_rssi();
            printf("LoRa: Received packet of size %d\n", len);
            printf("RSSI: %d dB\n", rssi);

            if (len == sizeof(payload_t))
            {
                payload_t *received_payload = (payload_t *)_rx_buff;
                printf("Header: 0x%02X\n", received_payload->header);
                printf("Node ID: 0x%04X\n", received_payload->node_id);
                printf("Timestamp: 0x%08lX\n", received_payload->timestamp);
                printf("Battery Level: 0x%02X\n", received_payload->battery_level);
                printf("Temperature: 0x%02X\n", received_payload->temperature);
                printf("Reduced FFT: ");
                for (int i = 0; i < sizeof(received_payload->reduced_fft); i++)
                {
                    printf("0x%02X ", received_payload->reduced_fft[i]);
                }
                printf("\n");
            }
            else
            {
                printf("Received packet size does not match payload size\n");
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
