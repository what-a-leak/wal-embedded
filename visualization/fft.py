import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial

# Configuration
SERIAL_PORT = '/dev/ttyACM0'  # Replace with your port
BAUD_RATE = 115200
BATCH_SIZE = 1024

def read_serial_data(serial_port, batch_size):
    """Read a batch of PCM data from the serial port."""
    data = []
    while len(data) < batch_size:
        try:
            line = serial_port.readline().decode('utf-8').strip()
            value = int(line)
            data.append(value)
        except ValueError:
            continue
    return np.array(data, dtype=np.int16)

def perform_fft(data, sample_rate):
    """Perform FFT on the data and calculate the frequency spectrum."""
    fft_result = np.fft.fft(data)
    freqs = np.fft.fftfreq(len(data), d=1/sample_rate)
    magnitude = np.abs(fft_result)  # Magnitude of FFT
    return freqs[:len(freqs)//2], magnitude[:len(magnitude)//2]  # Take positive frequencies only

def update_plot(frame, ser, line, sample_rate):
    """Update the plot with new data."""
    data = read_serial_data(ser, BATCH_SIZE)
    freqs, magnitude = perform_fft(data, sample_rate)
    line.set_data(freqs, magnitude)
    return line,

def main():
    # Open serial port
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Connected to {SERIAL_PORT}")

        # Set up the plot
        fig, ax = plt.subplots()
        line, = ax.plot([], [], lw=2)
        ax.set_xlim(50, 2000)  # Adjust x-axis limit to 50Hz to 2000Hz
        ax.set_ylim(0, 100000)  # Adjust y-axis limit based on expected amplitude
        ax.set_title('Frequency Spectrum')
        ax.set_xlabel('Frequency (Hz)')
        ax.set_ylabel('Amplitude')
        ax.grid(True)

        # Sample rate
        sample_rate = 44100  # Replace with the actual sample rate

        # Create animation
        ani = animation.FuncAnimation(fig, update_plot, fargs=(ser, line, sample_rate), interval=1000, blit=True)

        plt.show()

if __name__ == "__main__":
    main()
