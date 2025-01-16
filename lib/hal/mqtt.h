#ifndef HEADER_HAL_MQTT
#define HEADER_HAL_MQTT

void mqtt_init(const char* uri);
void mqtt_publish(const char* topic, const char* data);
int mqtt_check_connection();
int mqtt_check_publish();

#endif //HEADER_HAL_MQTT