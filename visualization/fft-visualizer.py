import serial
import numpy as np
import matplotlib.pyplot as plt

SERIAL_PORT = "/dev/ttyACM0" 
BAUD_RATE = 115200
NUM_SAMPLES = 512  # Number of FFT bins

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(np.zeros(NUM_SAMPLES))
ax.set_xlim(0, NUM_SAMPLES)
ax.set_ylim(-32, 100)
ax.set_title("FFT Spectrum")
ax.set_xlabel("Frequency Bin")
ax.set_ylabel("Magnitude (dB)")

try:
    while True:
        # Read a line of FFT data from the ESP32
        data = ser.readline().decode('utf-8').strip()
        if data:
            try:
                # Convert the CSV data to a NumPy array
                fft_data = np.array([float(x) for x in data.split(",") if x and x.count('.') <= 1], dtype=float)

                # Update the plot
                if len(fft_data) == NUM_SAMPLES:
                    line.set_ydata(fft_data)
                    plt.pause(0.01)
            except ValueError as e:
                print(f"Error parsing data: {e}")
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
