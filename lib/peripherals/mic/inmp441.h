#ifndef INMP_H
#define INMP_H

#include <stddef.h> 

#define I2S_NUM         I2S_NUM_0  // Default I2S port
#define SAMPLE_RATE     44100      // Default sampling rate
#define I2S_READ_LEN    128       // Default buffer length
#define GPIO_SCK        1          // Default GPIO for SCK (BCK)
#define GPIO_SD         2          // Default GPIO for SD
#define GPIO_WS         3          // Default GPIO for WS (LRCLK)

// Initialize INMP441 microphone with specified GPIO pins and I2S configuration
void inmp_init(int gpio_sck, int gpio_sd, int gpio_ws, int sample_rate);

// Read the sound level Root Mean Square (RMS) value
float inmp_read_sound_level();

#endif
