#include "inmp441.h"
#include <math.h>
#include "driver/i2s.h"
#include "esp_log.h"

static const char *TAG = "INMP";

void inmp_init(int gpio_sck, int gpio_sd, int gpio_ws, int sample_rate)
{
    // Configure I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,  // Master mode, receive
        .sample_rate = sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 16-bit data
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // Mono
        .communication_format = I2S_COMM_FORMAT_I2S,  // Standard I2S format
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level
        .dma_buf_count = 4,                           // Number of DMA buffers
        .dma_buf_len = I2S_READ_LEN / 2,              // DMA buffer length
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // Configure I2S pins
    i2s_pin_config_t pin_config = {
        .bck_io_num = gpio_sck,
        .ws_io_num = gpio_ws,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = gpio_sd
    };

    // Install and configure I2S
    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM, &pin_config));
    ESP_LOGI(TAG, "INMP Initialized");
}

float inmp_read_sound_level()
{
    int16_t i2s_read_buff[I2S_READ_LEN]; // Buffer for I2S data
    size_t bytes_read;
    float sum = 0;
    size_t samples_read;

    // Read data from I2S
    i2s_read(I2S_NUM, i2s_read_buff, sizeof(i2s_read_buff), &bytes_read, portMAX_DELAY);

    // Calculate RMS
    samples_read = bytes_read / sizeof(int16_t);
    for (size_t i = 0; i < samples_read; i++) {
        sum += i2s_read_buff[i] * i2s_read_buff[i];
    }

    return sqrt(sum / samples_read); // Return the RMS value
}

