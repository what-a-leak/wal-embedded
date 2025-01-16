#ifndef HEADER_DEMO_SEND
#define HEADER_DEMO_SEND

// Constants for FFT
#define SAMPLE_RATE 88200 // Fixed aliasing issue
#define N_SAMPLES 1024

void send_task();
void compute_fft_task();
void test_encryption_task();

#endif //HEADER_DEMO_SEND