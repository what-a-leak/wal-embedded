#ifndef HEADER_SCREEN
#define HEADER_SCREEN

#include <hal/gpio_types.h>
#include <stdint.h>

void screen_init(gpio_num_t scl, gpio_num_t sda, uint32_t speed);
void screen_printf(const uint8_t line, const char* format, ...);
void screen_draw(const uint8_t line, const char* str, int len);
void screen_clear();
void screen_update();

#endif //HEADER_SCREEN