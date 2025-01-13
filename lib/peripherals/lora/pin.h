#ifndef HEADER_PIN
#define HEADER_PIN

#include <driver/gpio.h>

#define SCREEN_SDA  GPIO_NUM_8
#define SCREEN_SCL  GPIO_NUM_9
#define BUTTON      GPIO_NUM_1

// See wal schematic on https://github.com/what-a-leak/wal-hardware
#define SX1278_PIN_MISO GPIO_NUM_5
#define SX1278_PIN_MOSI GPIO_NUM_6
#define SX1278_PIN_CLK  GPIO_NUM_4
#define SX1278_PIN_CS   GPIO_NUM_7
#define SX1278_PIN_DIO0 GPIO_NUM_10
#define SX1278_PIN_RST  GPIO_NUM_NC

#endif //HEADER_PIN