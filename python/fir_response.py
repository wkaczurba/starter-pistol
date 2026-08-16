"""
Plot the frequency response of the FIR filter(s) used in the firmware
(src/main.cpp / arduino/starter-pistol/starter-pistol.ino, function
fir_process). Coefficients are copied verbatim from the firmware so the
plot always reflects what actually runs on the device.

Usage:
    python3 fir_response.py
"""

import numpy as np
from scipy.signal import freqz
import matplotlib.pyplot as plt

SAMPLE_RATE = 16000  # Hz, matches SAMPLE_RATE in the firmware

# Short filter (FIR_LEN = 7): light smoothing / de-click. Active when
# USE_LONG_FIR is NOT defined in the firmware build.
h_short = np.array([
    -0.06, 0.00, 0.24, 0.64, 0.24, 0.00, -0.06,
])

# Long filter (FIR_LEN = 63): 3-6 kHz band-pass, fs = 16 kHz. This is the
# firmware's current default (USE_LONG_FIR is defined in main.cpp /
# starter-pistol.ino).
h_long = np.array([
    -0.000000000, -0.001022169, -0.000000000,  0.001808807,
     0.000000000, -0.003067968, -0.000000000,  0.004823591,
     0.000000000, -0.007093902, -0.000000000,  0.009894618,
     0.000000000, -0.013236624, -0.000000000,  0.017120627,
     0.000000000, -0.021536681, -0.000000000,  0.026464321,
     0.000000000, -0.031873220, -0.000000000,  0.037724560,
     0.000000000, -0.043971060, -0.000000000,  0.050558636,
     0.000000000, -0.057428697, -0.000000000,  0.314528000,
    -0.000000000, -0.057428697,  0.000000000,  0.050558636,
    -0.000000000, -0.043971060,  0.000000000,  0.037724560,
    -0.000000000, -0.031873220,  0.000000000,  0.026464321,
    -0.000000000, -0.021536681,  0.000000000,  0.017120627,
    -0.000000000, -0.013236624,  0.000000000,  0.009894618,
    -0.000000000, -0.007093902,  0.000000000,  0.004823591,
    -0.000000000, -0.003067968,  0.000000000,  0.001808807,
    -0.000000000, -0.001022169, -0.000000000,
])

FILTERS = {
    "short (FIR_LEN=7)": h_short,
    "long (FIR_LEN=63, USE_LONG_FIR, default)": h_long,
}


def plot_responses(filters: dict, fs: int) -> None:
    fig, (ax_mag, ax_phase) = plt.subplots(2, 1, figsize=(9, 7), sharex=True)

    for label, h in filters.items():
        w, hf = freqz(h, worN=4096, fs=fs)
        mag_db = 20 * np.log10(np.maximum(np.abs(hf), 1e-12))
        phase = np.unwrap(np.angle(hf))

        ax_mag.plot(w, mag_db, label=label)
        ax_phase.plot(w, phase, label=label)

    ax_mag.set_title(f"FIR filter frequency response (fs = {fs} Hz)")
    ax_mag.set_ylabel("Magnitude (dB)")
    ax_mag.grid(True, which="both", alpha=0.3)
    ax_mag.legend()

    ax_phase.set_xlabel("Frequency (Hz)")
    ax_phase.set_ylabel("Phase (rad)")
    ax_phase.grid(True, which="both", alpha=0.3)
    ax_phase.legend()

    fig.tight_layout()

    out_path = "fir_response.png"
    fig.savefig(out_path, dpi=150)
    print(f"Saved plot to {out_path}")

    plt.show()


if __name__ == "__main__":
    plot_responses(FILTERS, SAMPLE_RATE)
