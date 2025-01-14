import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
COMPRESSED_SIZE = 20
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
    cmap="inferno",
    origin="lower",
)
fig.colorbar(cax, ax=ax, label="Amplitude (dB)")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Time (Frames)")
ax.set_title("Audio Spectrogram")

dynamic_min = None
dynamic_max = None

def decompress_fft(compressed_data):
    """Décompresse les données FFT."""
    if len(compressed_data) != COMPRESSED_SIZE:
        raise ValueError(f"Invalid compressed data size: {len(compressed_data)}")

    # Récupération de min_val et max_val
    min_val = (compressed_data[0] / 2) - 128
    max_val = (compressed_data[1] / 2) - 128

    # Décompression des données normalisées
    fft_data = np.array(compressed_data[2:], dtype=np.float32) / 255.0
    fft_data = fft_data * (max_val - min_val) + min_val

    return fft_data

def read_serial_data():
    global dynamic_min, dynamic_max
    while ser.in_waiting:
        try:
            # Lire une ligne de données hexadécimales
            line = ser.readline().decode().strip()
            compressed_data = [int(line[i:i+2], 16) for i in range(0, len(line), 2)]

            # Décompresser les données FFT
            fft_data = decompress_fft(compressed_data)

            if len(fft_data) != freq_limit_idx:
                print(f"Unexpected FFT data length: {len(fft_data)}")
                return None

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
