import serial
import numpy as np
import matplotlib.pyplot as plt

# Configuration
SERIAL_PORT = "/dev/ttyACM0"  
BAUD_RATE = 115200
NUM_SAMPLES = 512  # Number of FFT bins
SAMPLE_RATE = 44100  # Sampling rate
MAX_FREQ = 20000  # Focus on frequencies up to 10 kHz

# Calculate frequency values for the x-axis
freqs = np.fft.rfftfreq(NUM_SAMPLES * 2, d=1.0 / SAMPLE_RATE)[:NUM_SAMPLES]

# Filter frequencies up to MAX_FREQ
focus_indices = freqs <= MAX_FREQ
focused_freqs = freqs[focus_indices]

# Initialize serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(focused_freqs, np.zeros_like(focused_freqs))
fig.set_size_inches(12.8, 7.2)
ax.set_xlim(0, MAX_FREQ)  # Limit x-axis to 0 Hz - 10 kHz
ax.set_ylim(-32, 100)  # Adjust based on expected FFT magnitude
ax.set_title("FFT Spectrum")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Magnitude (dB)")

try:
    while True:
        data = ser.readline().decode('utf-8').strip()
        if data:
            try:
                # Convert the CSV data to a NumPy array
                fft_data = np.array([float(x) for x in data.split(",") if x and x.count('.') <= 1], dtype=float)

                # Update the plot if the data length matches
                if len(fft_data) == NUM_SAMPLES:
                    # Focus only on the selected frequency range
                    focused_fft_data = fft_data[focus_indices]
                    line.set_ydata(focused_fft_data)
                    plt.pause(0.01)
            except ValueError as e:
                print(f"Error parsing data: {e}")
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
