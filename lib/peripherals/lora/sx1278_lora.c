#include "sx1278_lora.h"
#include "sx127x.h"
#include "pin.h"

#include <driver/spi_master.h>
#include <freertos/task.h>

#define DATA_SIZE   2
#define TIMEOUT     100

// Private members
static spi_device_handle_t spi;
static uint8_t rx_data[DATA_SIZE] = {0};
static uint8_t tx_data[DATA_SIZE] = {0};
static uint64_t _frequency = 0;

static inline uint16_t spi_read(uint8_t reg)
{
    tx_data[0] = reg & 0x7F;
    tx_data[1] = 0;
    spi_transaction_t transaction = {
        .length = DATA_SIZE*8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    if(spi_device_transmit(spi, &transaction) != ESP_OK)
        return 0;
    else
        return rx_data[1];
}

static inline void spi_write(uint8_t reg, uint8_t data)
{
    tx_data[0] = 0x80 | reg;
    tx_data[1] = data;
    spi_transaction_t transaction = {
        .length = DATA_SIZE*8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    spi_device_transmit(spi, &transaction);
}

esp_err_t lora_init(void) {
    esp_err_t ret = ESP_OK;

    /* Reset in case of issues */
    if(SX1278_PIN_RST != GPIO_NUM_NC)
    {
        gpio_reset_pin(SX1278_PIN_RST);
        gpio_set_direction(SX1278_PIN_RST, GPIO_MODE_OUTPUT);
    }
    /* Chip Select set to output */
    if(SX1278_PIN_CS != GPIO_NUM_NC)
    {
        gpio_reset_pin(SX1278_PIN_CS);
        gpio_set_direction(SX1278_PIN_CS, GPIO_MODE_OUTPUT);
        gpio_set_level(SX1278_PIN_CS, 1);
    }

    const spi_bus_config_t buscfg = {
        .miso_io_num = SX1278_PIN_MISO,
        .mosi_io_num = SX1278_PIN_MOSI,
        .sclk_io_num = SX1278_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };
    const spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_MASTER_FREQ_9M, 
        .mode = 0,                        
        .spics_io_num = SX1278_PIN_CS,
        .queue_size = 7,
        .flags = 0,
        .pre_cb = NULL       
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
        return ret;
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    if (ret != ESP_OK)
        return ret;

    /* Reset in case of issues */
    lora_reset();

    /* Check if the version corresponds (device ok) */
    uint8_t major=0, minor=0;
    uint32_t timeout=0;
    while((timeout < TIMEOUT) && !(major == 1 && minor == 2))
    {
        lora_version(&major, &minor);
        WAL_PRINT("LoRa: Ver.%d.%d\n", major, minor);
        vTaskDelay(10);
        timeout++;
    }
    if(timeout >= TIMEOUT)
        return ESP_ERR_TIMEOUT;

    /* Default configuration */
    /* Sleep Mode */
    spi_write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    spi_write(REG_FIFO_RX_BASE_ADDR, 0);
    spi_write(REG_FIFO_TX_BASE_ADDR, 0);
    spi_write(REG_LNA, spi_read(REG_LNA) | 0x03);
    spi_write(REG_MODEM_CONFIG_3, 0x04);
    lora_set_tx_power(17);
    /* Idle Mode */
    spi_write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    
    return ret;
}

void lora_reset(void)
{
   gpio_set_level(SX1278_PIN_RST, 0);
   vTaskDelay(pdMS_TO_TICKS(1));
   gpio_set_level(SX1278_PIN_RST, 1);
   vTaskDelay(pdMS_TO_TICKS(10));
}

esp_err_t lora_version(uint8_t* major, uint8_t* minor)
{
    const int16_t res = spi_read(REG_VERSION);
    if(res < 0)
        return ESP_FAIL;
    *minor = res & 0b1111;
    *major = (res & 0b11110000) >> 4;
    return ESP_OK;
}

void lora_set_tx_power(int level)
{
   if (level < 2) level = 2;
   else if (level > 17) level = 17;
   spi_write(REG_PA_CONFIG, 0x80 | (level - 2));
}

esp_err_t lora_set_frequency(long freq)
{
   _frequency = freq;
   uint64_t frf = ((uint64_t)freq << 19) / 32000000;

   spi_write(REG_FRF_MSB, (uint8_t)(frf >> 16));
   spi_write(REG_FRF_MID, (uint8_t)(frf >> 8));
   spi_write(REG_FRF_LSB, (uint8_t)(frf >> 0));
   return ESP_OK;
}

esp_err_t lora_enable_crc(void)
{
   spi_write(REG_MODEM_CONFIG_2, spi_read(REG_MODEM_CONFIG_2) | 0x04);
   return ESP_OK;
}

esp_err_t lora_set_coding_rate(int denominator)
{
    if (denominator < 5)
        denominator = 5;
    else if (denominator > 8)
        denominator = 8;

    spi_write(REG_MODEM_CONFIG_1, (spi_read(REG_MODEM_CONFIG_1) & 0xf1) | ((denominator - 4) << 1));
    return ESP_OK;
}

esp_err_t lora_set_bandwidth(int bandwidth)
{
    if (bandwidth < 10)
        spi_write(REG_MODEM_CONFIG_1, (spi_read(REG_MODEM_CONFIG_1) & 0x0f) | (bandwidth << 4));
    return ESP_OK;
}

esp_err_t lora_set_spreading_factor(int spreading_factor)
{
   if (spreading_factor < 6)
    spreading_factor = 6;
   else if (spreading_factor > 12)
    spreading_factor = 12;

   if (spreading_factor == 6) {
      spi_write(REG_DETECTION_OPTIMIZE, 0xc5);
      spi_write(REG_DETECTION_THRESHOLD, 0x0c);
   } else {
      spi_write(REG_DETECTION_OPTIMIZE, 0xc3);
      spi_write(REG_DETECTION_THRESHOLD, 0x0a);
   }

   spi_write(REG_MODEM_CONFIG_2, (spi_read(REG_MODEM_CONFIG_2) & 0x0f) | ((spreading_factor << 4) & 0xf0));
    return ESP_OK;
}

int lora_receive()
{
    spi_write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
    return 0;
}

int lora_received()
{
    if(spi_read(REG_IRQ_FLAGS) & IRQ_RX_DONE_MASK) return 1;
    return 0;
}

int lora_receive_packet(uint8_t *buf, int size)
{
    int irq = spi_read(REG_IRQ_FLAGS), len = 0;

    /* Clear the IRQ flags */
    spi_write(REG_IRQ_FLAGS, irq);

    /* Either CRC error or RX not done -> return len of 0 */
    if ((irq & IRQ_RX_DONE_MASK) == 0)
        return len;
    if (irq & IRQ_PAYLOAD_CRC_ERROR_MASK)
        return len;

    /* Length of the packet */
    len = spi_read(REG_RX_NB_BYTES);

    /* Set FIFO pointer to the current RX address */
    uint8_t currentAddr = spi_read(REG_FIFO_RX_CURRENT_ADDR);
    spi_write(REG_FIFO_ADDR_PTR, currentAddr);

    /* Read the payload from the FIFO */
    for (int i = 0; i < len && i < size; i++) {
        buf[i] = spi_read(REG_FIFO);
    }

    return len;
}


esp_err_t lora_send_packet(uint8_t *buf, int size)
{
    /* Double checking if in idle mode */
    spi_write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    spi_write(REG_FIFO_ADDR_PTR, 0);

    /* Sending data to the FIFO register */
    for(int i=0; i<size; i++) 
      spi_write(REG_FIFO, buf[i]);
    spi_write(REG_PAYLOAD_LENGTH, size);

    /* Switching to TX mode */
    spi_write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
    
    /* Checking if data has been transfered */
    uint32_t timeout = 0;
    int irq = 0;
    while((timeout < TIMEOUT) && !(irq & IRQ_TX_DONE_MASK))
    {
        irq = spi_read(REG_IRQ_FLAGS);
        vTaskDelay(10);
        timeout++;
    }

   /* Idle mode + IRQ set to TX success */
    spi_write(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
    spi_write(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    if(timeout >= TIMEOUT)
        return ESP_ERR_TIMEOUT;
    else
        return ESP_OK;
}

int lora_get_coding_rate(void)
{
    return ((spi_read(REG_MODEM_CONFIG_1) & 0x0E) >> 1);
}

int lora_get_bandwidth(void)
{
    return ((spi_read(REG_MODEM_CONFIG_1) & 0xf0) >> 4);
}

int lora_get_spreading_factor(void)
{
    return (spi_read(REG_MODEM_CONFIG_2) >> 4);
}

long lora_get_frequency(void)
{
    uint64_t frequency = ((uint64_t)((spi_read(REG_FRF_MSB) << 16) + (spi_read(REG_FRF_MID) << 8) + spi_read(REG_FRF_LSB))*32000000) >> 19;
    return frequency;
}

int lora_packet_rssi(void)
{
   return (spi_read(REG_PKT_RSSI_VALUE) - (_frequency < 868e6 ? 164 : 157));
}

esp_err_t lora_clean(void)
{
    esp_err_t ret = spi_bus_remove_device(spi);
    if (ret != ESP_OK)
        return ret;
    ret = spi_bus_free(SPI2_HOST);
    return ret;
}
