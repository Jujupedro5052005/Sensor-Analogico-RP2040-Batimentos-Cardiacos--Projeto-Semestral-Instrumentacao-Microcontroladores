# !pip install pyserial matplotlib numpy
import serial
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque
import numpy as np

FILTER_WINDOW = 2

FINGER_THRESHOLD = 1200
MIN_SAMPLES = 50

PEAK_WINDOW = 50

SAMPLE_RATE = 100

BUFFER_WINDOW = 1

THRESHOLD_OFFSET = 0

last_peak_idx = None
bpm = 0
bpm_candidate = 0
bpm_history = deque(maxlen=5)

candidate_peak_value = 0
candidate_peak_idx = 0

samples_since_candidate = 0

valid_count = 0
has_finger = False

filter_buffer = deque(maxlen=FILTER_WINDOW)

threshold = FINGER_THRESHOLD

buffer = deque(maxlen=BUFFER_WINDOW)

# =========================
# SERIAL
# =========================

PORT = "COM4"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

# =========================
# BUFFER
# =========================

N = 500

signal_data = deque([0] * N, maxlen=N)

peak_x = deque(maxlen=100)
peak_y = deque(maxlen=100)

sample_counter = 0

# =========================
# PLOT
# =========================

fig, ax = plt.subplots(figsize=(12, 6))

line, = ax.plot(range(N), signal_data, label="Signal")

threshold_line = ax.axhline(
    FINGER_THRESHOLD,
    linestyle="--",
    linewidth=1,
    color="red",
    label="Finger Threshold"
)

peak_scatter = ax.scatter(
    [],
    [],
    marker="o",
    s=50,
    label="Detected Peaks"
)

ax.set_xlabel("Samples")
ax.set_ylabel("ADC Value")
ax.set_title("Raw Pico ADC Signal")

ax.legend()

# =========================
# UPDATE
# =========================

def update(frame):

    global sample_counter

    while ser.in_waiting:

        try:
            line_str = ser.readline().decode("utf-8").strip()

            value = float(line_str)

            value = max(min(value,4095),0)

            filter_buffer.append(value)

            filtered = sum(filter_buffer) / len(filter_buffer)

            buffer.append(filtered)

            if len(buffer) == BUFFER_WINDOW:
                moving_average = sum(buffer) / BUFFER_WINDOW

                threshold = moving_average + THRESHOLD_OFFSET

            global valid_count, has_finger
            
            if FINGER_THRESHOLD < filtered < 4095:
                valid_count += 1
            else:
                valid_count = 0
            
            has_finger = valid_count >= MIN_SAMPLES

            signal_data.append(filtered)
        

            # -------------------------
            # Peak detector
            # -------------------------

            global candidate_peak_value
            global candidate_peak_idx
            global samples_since_candidate

            if len(signal_data) >= 3:
            
                a = signal_data[-3]
                b = signal_data[-2]
                c = signal_data[-1]

                # máximo local encontrado
                if b > a and b > c:
                
                    # guarda somente o maior da janela
                    if b > candidate_peak_value:
                    
                        candidate_peak_value = b
                        candidate_peak_idx = sample_counter - 1

            # conta amostras desde o último pico aceito
            samples_since_candidate += 1

            # terminou a janela?
            if samples_since_candidate >= PEAK_WINDOW and b > threshold:
            
                # aceita apenas o maior pico da janela
                if candidate_peak_value > 0:
                
                    peak_x.append(candidate_peak_idx)
                    peak_y.append(candidate_peak_value)

                    global last_peak_idx, bpm

                    if last_peak_idx is not None:

                        delta_samples = candidate_peak_idx - last_peak_idx

                        delta_seconds = delta_samples/SAMPLE_RATE

                        if delta_seconds > 0:
                            bpm_candidate = 60.0 /delta_seconds

                            if 30 <= bpm_candidate <= 200:
                                bpm_history.append(bpm_candidate)

                                bpm = sum(bpm_history) / len(bpm_history)

                        print(f"BPM = {bpm:.1f}")

                    last_peak_idx = candidate_peak_idx

                # reinicia janela
                candidate_peak_value = 0
                candidate_peak_idx = 0
                samples_since_candidate = 0

            sample_counter += 1

        except Exception:
            pass

    # Atualiza sinal
    line.set_ydata(signal_data)

    # Atualiza picos
    visible_peaks_x = []
    visible_peaks_y = []

    start_idx = sample_counter - len(signal_data)

    for x, y in zip(peak_x, peak_y):
        if x >= start_idx:
            visible_peaks_x.append(x - start_idx)
            visible_peaks_y.append(y)

    peak_scatter.set_offsets(
        np.column_stack((visible_peaks_x, visible_peaks_y))
        if visible_peaks_x
        else np.empty((0, 2))
    )

    sig_min = min(signal_data)
    sig_max = max(signal_data)

    margin = max(10, (sig_max - sig_min) * 0.2)

    ax.set_ylim(
        sig_min - margin,
        sig_max + margin
    )

    return line, peak_scatter

# =========================
# ANIMATION
# =========================

ani = FuncAnimation(
    fig,
    update,
    interval=20,
    blit=False,
    cache_frame_data=False
)

plt.tight_layout()
plt.show()