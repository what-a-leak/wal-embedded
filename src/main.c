#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef MASTER_NODE
#include "recv.h"
#else
#include "send.h"
#endif

void app_main() {
    #ifdef MASTER_NODE
    xTaskCreate(send_mqtt, "send_mqtt", 4000, NULL, 5, NULL);
    //xTaskCreate(recv_task, "receive_task", 4000, NULL, 5, NULL);
    #else
    xTaskCreate(compute_fft_task, "compute_fft_task", 12000, NULL, 5, NULL);
    xTaskCreate(send_task, "send_task", 4000, NULL, 5, NULL); 
    #endif
}
