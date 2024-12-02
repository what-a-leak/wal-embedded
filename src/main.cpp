#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define BLINK_GPIO GPIO_NUM_2
#define ALPHA 0.99 // Smoothing factor for the high-pass filter

extern "C" void app_main(void)
{
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // Adjust attenuation

    float dc_offset = 0; // To store the estimated DC offset
    while (true)
    {
        // Read ADC value
        int adc_value = adc1_get_raw(ADC1_CHANNEL_0);

        // High-pass filter to remove DC offset
        dc_offset = ALPHA * dc_offset + (1 - ALPHA) * adc_value;
        int adc_value_no_dc = adc_value - (int)dc_offset;

        printf("%d\n", adc_value_no_dc);

        vTaskDelay(pdMS_TO_TICKS(4));
    }
}
