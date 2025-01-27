#ifndef HEADER_DEMO_PROTOCOL
#define HEADER_DEMO_PROTOCOL

#include "../lib/peripherals/lora/sx127x.h"
#include "../lib/peripherals/lora/sx1278_lora.h"

// Constants for LoRa Communication
#define CODING_RATE         8   // CR = 4/8
#define BANDWIDTH           7   // 125 kHz from the datasheet
#define SPREADING_FACTOR    10  // SF10
#define MAX_PACKET_SIZE     256

// Payload structure: Header (8 bit), Node ID (16 bit), Timestamp (32 bit), Battery Level (8 bit), Temperature (8 bit), Reduced_FFT (184 bit)
typedef struct {
    uint8_t header;
    uint16_t node_id;
    uint32_t timestamp;
    uint8_t battery_level;
    uint8_t temperature;
    uint8_t reduced_fft[128]; // 184 bits / 8 bits per byte = 23 bytes
} payload_t;

static inline void protocol_set_payload(payload_t *p, uint8_t *data) {
    payload_t *temp = (payload_t *)data;
    *p = *temp;
}

#endif //HEADER_DEMO_PROTOCOL