#include "mqtt.h"
#include <mqtt_client.h>

esp_mqtt_client_handle_t _client;
int _connected = 0;
int _actual_msg_id = -1;

static void mqtt_event_handler(void *event_handler_arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    const esp_mqtt_event_id_t event_mqtt_id = event->event_id;

    if(event_mqtt_id == MQTT_EVENT_CONNECTED || event_mqtt_id == MQTT_EVENT_DISCONNECTED)
        _connected = (event->event_id == MQTT_EVENT_CONNECTED);
    if(event_mqtt_id == MQTT_EVENT_PUBLISHED)
        _actual_msg_id = event->msg_id;
}

void mqtt_init(const char *uri)
{
    esp_mqtt_client_config_t config = {
        .broker.address.uri = uri,
    };

    _client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(_client, ESP_EVENT_ANY_ID, mqtt_event_handler, _client);
    esp_mqtt_client_start(_client);
}

void mqtt_publish(const char *topic, const char *data)
{
    int msg_id = esp_mqtt_client_publish(_client, topic, data, 0, 1, 0);
    _actual_msg_id = msg_id;
}

int mqtt_check_connection()
{
    return _connected;
}

int mqtt_check_publish()
{
    int temp = _actual_msg_id;
    _actual_msg_id = -1;
    return temp;
}