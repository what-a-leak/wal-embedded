#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define BLINK_GPIO GPIO_NUM_2
#define SAMPLE_COUNT 100

extern "C" void app_main(void)
{
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // Adjust attenuation

    int adc_values[SAMPLE_COUNT] = {0};
    int index = 0;
    int sum = 0;

    while (true)
    {
        // Read ADC value
        int adc_value = adc1_get_raw(ADC1_CHANNEL_0);

        // Update the sum and the circular buffer
        sum -= adc_values[index];
        adc_values[index] = adc_value;
        sum += adc_value;
        index = (index + 1) % SAMPLE_COUNT;

        // Calculate the average
        int average = sum / SAMPLE_COUNT;

        // Remove DC offset
        int adc_value_no_dc = adc_value - average;

        printf("Electret Microphone Value (No DC): %d\n", adc_value_no_dc);

        vTaskDelay(pdMS_TO_TICKS(4));
    }
}