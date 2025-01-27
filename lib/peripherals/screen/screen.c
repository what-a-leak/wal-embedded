#include "ssd1306.h"
// ! To be replaced with:
// #include <esp_lcd_panel_ssd1306.h>

#include "font8x8_basic.h"
#include <stdarg.h>

#define I2C_MASTER_NUM          I2C_NUM_0
#define MAX_CHAR                16

static ssd1306_handle_t ssd1306_dev = NULL;

void screen_init(gpio_num_t scl, gpio_num_t sda, uint32_t speed)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .master.clk_speed = speed,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL};
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
    ssd1306_refresh_gram(ssd1306_dev);
    ssd1306_clear_screen(ssd1306_dev, 0x00);
}

inline static void ssd1306_draw_bitmap_8bit(ssd1306_handle_t dev, uint8_t chXpos, uint8_t chYpos,
                         const uint8_t *pchBmp)
{
    uint16_t i, j;

    for(i=0; i<8; i++)
    {
        for(j=8; j>0; j--)
        {
            if((pchBmp[i] >> (j-1)) & 0x1)
                ssd1306_fill_point(dev, chXpos + j, chYpos + i, 1);
        }
    }
}

void screen_printf(const uint8_t line,const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buff[16] = {0}; 
    int len = vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);

    if (len > 0) {
        int x_pos = 0;
        for (int i = 0; i < len; i++) {
            const uint8_t char_index = buff[i];
            ssd1306_draw_bitmap_8bit(ssd1306_dev, x_pos, (line%8)*8, (const uint8_t*)(font8x8_basic[char_index]));
            x_pos += 8;
        }
    }
    ssd1306_refresh_gram(ssd1306_dev);
}

void screen_draw(const uint8_t line, const char* str, int len)
{
    int x_pos = 0;
    for (int i = 0; i < len; i++) {
        const uint8_t char_index = str[i];
        ssd1306_draw_bitmap_8bit(ssd1306_dev, x_pos, (line%8)*8, (const uint8_t*)(font8x8_basic[char_index]));
        x_pos += 8;
    }
}

void screen_clear()
{
    ssd1306_refresh_gram(ssd1306_dev);
    ssd1306_clear_screen(ssd1306_dev, 0x00);
}

void screen_update()
{
    ssd1306_refresh_gram(ssd1306_dev);
}