import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
NUM_SAMPLES = 1024
SAMPLE_RATE = 88200
MAX_DISPLAY_FREQ = 22000

frequencies = np.linspace(0, SAMPLE_RATE / 2, NUM_SAMPLES // 2)
freq_limit_idx = int((MAX_DISPLAY_FREQ / (SAMPLE_RATE / 2)) * (NUM_SAMPLES // 2))
frequencies = frequencies[:freq_limit_idx]

ser = serial.Serial(SERIAL_PORT, BAUD_RATE)

time_steps = 100
spectrogram_data = np.zeros((time_steps, freq_limit_idx))

fig, ax = plt.subplots()
cax = ax.imshow(
    spectrogram_data,
    extent=[0, MAX_DISPLAY_FREQ, 0, time_steps],
    aspect="auto",
    cmap="inferno", # viridis
    origin="lower",
)
fig.colorbar(cax, ax=ax, label="Amplitude (dB)")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Time (Frames)")
ax.set_title("Audio Spectrogram")

dynamic_min = None
dynamic_max = None

def read_serial_data():
    global dynamic_min, dynamic_max
    while ser.in_waiting:
        try:
            data = ser.readline().decode().strip()
            fft_data = np.array([float(x) for x in data.split(",") if x])
            if len(fft_data) != NUM_SAMPLES // 2:
                print(f"Unexpected FFT data length: {len(fft_data)}")
                return None

            fft_data = fft_data[:freq_limit_idx]

            if dynamic_min is None or dynamic_max is None:
                dynamic_min = np.min(fft_data)
                dynamic_max = np.max(fft_data)
            else:
                dynamic_min = min(dynamic_min, np.min(fft_data))
                dynamic_max = max(dynamic_max, np.max(fft_data))

            return fft_data
        except Exception as e:
            print(f"Error parsing data: {e}")
            return None

def update(frame):
    global spectrogram_data
    fft_data = read_serial_data()
    if fft_data is not None:
        spectrogram_data = np.roll(spectrogram_data, -1, axis=0)
        spectrogram_data[-1, :] = fft_data

        cax.set_clim(dynamic_min, dynamic_max)
        cax.set_data(spectrogram_data)
    return cax,

ani = FuncAnimation(fig, update, blit=True, interval=100)

plt.show()

ser.close()
