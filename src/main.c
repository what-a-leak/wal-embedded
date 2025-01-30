#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef MASTER_NODE
#include "recv.h"
#else
#include "send.h"
#endif

#ifndef USE_SCREEN
#define screen_log(...)
#else
#include "../lib/peripherals/lora/pin.h"
#include "../lib/peripherals/screen/screen.h"
#include "../lib/peripherals/screen/logger.h"
#endif

void app_main() {
    // Screen Init
    #ifdef USE_SCREEN
    screen_init(SCREEN_SCL,SCREEN_SDA,200000);
    screen_log("Screen: Init.");
    #endif

    // Communication
    #ifdef MASTER_NODE
    screen_log("Mode: Master");
    xTaskCreate(recv_task, "receive_task", 4000, NULL, 5, NULL);
    xTaskCreate(send_mqtt, "send_mqtt", 4000, NULL, 5, NULL);
    #else
    screen_log("Mode: Node");
    xTaskCreate(compute_fft_task, "compute_fft_task", 12000, NULL, 5, NULL);
    // xTaskCreate(send_task, "send_task", 4000, NULL, 5, NULL); 
    #endif

    // Encryption
    #ifndef DISABLE_SECURITY
    xTaskCreate(test_encryption_task, "test_encryption_task", 4000, NULL, 5, NULL); 
    #endif
}
