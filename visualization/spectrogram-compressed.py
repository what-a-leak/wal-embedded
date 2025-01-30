import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# === Configuration de la connexion série ===
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200

# === Paramètres de la FFT ===
NUM_COMPRESSED_SAMPLES = 22  # Taille compressée
SAMPLE_RATE = 88200
MAX_DISPLAY_FREQ = 22000

# === Fréquences correspondant aux 22 échantillons ===
frequencies = np.linspace(0, MAX_DISPLAY_FREQ, NUM_COMPRESSED_SAMPLES)

# === Initialisation du port série ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)

# === Paramètres du spectrogramme ===
time_steps = 100
spectrogram_data = np.zeros((time_steps, NUM_COMPRESSED_SAMPLES))

# === Création de la figure Matplotlib ===
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
ax.set_title("Audio Spectrogram (Compressed)")

# === Définition des bornes dynamiques de l'affichage ===
dynamic_min = None
dynamic_max = None

def decompress_fft(compressed_data):
    """
    Décompresse les valeurs FFT reçues en appliquant l’inverse de la transformation logarithmique.
    """
    compressed_data = np.array(compressed_data, dtype=np.uint8)  # Convertir en numpy array
    norm_values = compressed_data / 255.0  # Re-normaliser entre 0 et 1
    fft_values = (np.expm1(norm_values * np.log1p(1.0)))  # Inverse de la compression logarithmique
    return fft_values * 100  # Échelle arbitraire pour ajuster l'affichage

def read_serial_data():
    global dynamic_min, dynamic_max
    while ser.in_waiting:
        try:
            data = ser.readline().decode().strip()
            compressed_data = [int(x) for x in data.split(",") if x]

            if len(compressed_data) != NUM_COMPRESSED_SAMPLES:
                print(f"Unexpected compressed data length: {len(compressed_data)}")
                return None

            fft_data = decompress_fft(compressed_data)

            # Mise à jour des bornes dynamiques
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

# === Lancement de l'animation ===
ani = FuncAnimation(fig, update, blit=True, interval=100)

plt.show()

ser.close()
