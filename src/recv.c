#include "recv.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint8_t _rx_buff[MAX_PACKET_SIZE] = {0};

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