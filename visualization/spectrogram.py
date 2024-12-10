import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import sys

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200
NUM_SAMPLES = 512  # Number of FFT bins
SAMPLE_RATE = 44100  # Sampling rate
MAX_FREQ = 20000

# Calculate frequency values for the x-axis
freqs = np.fft.rfftfreq(NUM_SAMPLES * 2, d=1.0 / SAMPLE_RATE)[:NUM_SAMPLES]

# Filter frequencies up to MAX_FREQ
focus_indices = freqs <= MAX_FREQ
focused_freqs = freqs[focus_indices]

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

plt.ion()
fig, ax = plt.subplots()
fig.set_size_inches(12.8, 7.2)

spectrogram_data = []

ax.set_xlim(0, MAX_FREQ)
ax.set_ylim(0, 100)  
ax.set_title("Audio Spectrogram")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Time (Frames)")
spectrogram_image = ax.imshow(
    np.zeros((100, len(focused_freqs))),  #
    extent=[0, MAX_FREQ, 0, 100],
    aspect='auto',
    origin='lower',
    cmap='inferno',
    norm=LogNorm(vmin=1e-2, vmax=1e2) 
)

# Color bar
cbar = plt.colorbar(spectrogram_image, ax=ax)
cbar.set_label("Magnitude (Log Scale)")

try:
    while True:
        data = ser.readline().decode('utf-8').strip()
        if data:
            try:
                # Convert the CSV data to a NumPy array
                fft_data = np.array([float(x) for x in data.split(",") if x and x.count('.') <= 1], dtype=float)

                # Process if data length matches NUM_SAMPLES
                if len(fft_data) == NUM_SAMPLES:
                    # Focus only on the selected frequency range
                    focused_fft_data = fft_data[focus_indices]

                    # Append new row to spectrogram data
                    spectrogram_data.append(focused_fft_data)

                    # Maintain a rolling window of the last 100 frames
                    if len(spectrogram_data) > 100:
                        spectrogram_data.pop(0)

                    # Update the spectrogram image
                    spectrogram_image.set_data(np.array(spectrogram_data))
                    spectrogram_image.set_extent([0, MAX_FREQ, 0, len(spectrogram_data)])
                    plt.pause(0.01)
            except ValueError as e:
                print(f"Error parsing data: {e}")
except KeyboardInterrupt:
    print("Exiting...")
    sys.exit(0)
finally:
    ser.close()
