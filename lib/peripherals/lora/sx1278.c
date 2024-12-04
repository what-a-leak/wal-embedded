#include <stdio.h>
#include "sx1278.h"

void check_sx1278(void) {
    esp_err_t ret;

    for (int i=0; i<3; i++) {
        printf("temporiser pour laisser un peu de temps aux modules de s'init\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    printf("Initializing SPI bus...\n");

    spi_bus_config_t buscfg = {
        .miso_io_num = SX1278_PIN_NUM_MISO,
        .mosi_io_num = SX1278_PIN_NUM_MOSI,
        .sclk_io_num = SX1278_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, 
        .mode = 0,                        
        .spics_io_num = SX1278_PIN_NUM_CS,// Chip select pin
        .queue_size = 1,                  
    };

    // Initialize SPI Bus
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        printf("Failed to initialize SPI bus: %s\n", esp_err_to_name(ret));
        return;
    }
    printf("SPI bus initialized successfully.\n");

    // Attach Device to SPI Bus
    spi_device_handle_t spi;
    printf("Adding SPI device...\n");
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    if (ret != ESP_OK) {
        printf("Failed to add SPI device: %s\n", esp_err_to_name(ret));
        return;
    }
    printf("SPI device added successfully.\n");

    // SPI Test Transaction
    uint8_t tx_data[2] = {0x42, 0x00}; // dummy register/data
    uint8_t rx_data[2] = {0};

    spi_transaction_t t = {
        .length = 16,        // total lenght in bit
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    printf("Starting SPI transaction...\n");
    printf("Transmitting: 0x%02X 0x%02X\n", tx_data[0], tx_data[1]);

    // Transmit Data
    ret = spi_device_transmit(spi, &t);
    if (ret == ESP_OK) {
        printf("Transaction successful.\n");
        printf("Received data: 0x%02X 0x%02X\n", rx_data[0], rx_data[1]);
    } else {
        printf("Transaction failed: %s\n", esp_err_to_name(ret));
    }

    printf("Cleaning up SPI resources...\n");

    // Cleanup
    ret = spi_bus_remove_device(spi);
    if (ret != ESP_OK) {
        printf("Failed to remove SPI device: %s\n", esp_err_to_name(ret));
    } else {
        printf("SPI device removed successfully.\n");
    }

    ret = spi_bus_free(SPI2_HOST);
    if (ret != ESP_OK) {
        printf("Failed to free SPI bus: %s\n", esp_err_to_name(ret));
    } else {
        printf("SPI bus freed successfully.\n");
    }

    printf("SPI check completed.\n");
}
