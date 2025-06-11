# The code below was created by ChatGPT based on an example in C given in:
# https://stackoverflow.com/questions/44059100/lightweight-bandpass-filter-for-embedded-device-no-fft
import math
import random

# Constants
pi = 3.141592
pi2 = 2.0 * pi
s = 48000.0  # Sample rate

def bandpass_filter(f_hz, bw_hz, x):
    """
    Bandpass IIR filter using the difference equation approach (pure Python).

    Parameters:
        f_hz (float): Filter center frequency in Hz.
        bw_hz (float): Filter bandwidth in Hz.
        x (list of float): Input signal samples.

    Returns:
        list of float: Filtered output samples.
    """
    n = len(x)
    y = [0.0] * n

    # Delayed samples
    x_2 = 0.0
    x_1 = 0.0
    y_1 = 0.0
    y_2 = 0.0

    f = f_hz / s
    bw = bw_hz / s

    R = 1 - (3 * bw)
    Rsq = R * R
    cosf2 = 2 * math.cos(pi2 * f)

    K = (1 - R * cosf2 + Rsq) / (2 - cosf2)

    a0 = 1.0 - K
    a1 = 2 * (K - R) * cosf2
    a2 = Rsq - K
    b1 = 2 * R * cosf2
    b2 = -Rsq

    for i in range(n):
        # IIR difference equation
        y[i] = a0 * x[i] + a1 * x_1 + a2 * x_2 + b1 * y_1 + b2 * y_2

        # Shift delayed samples
        x_2 = x_1
        x_1 = x[i]
        y_2 = y_1
        y_1 = y[i]

    return y

def run():
    white_noise = [random.randrange(-100000, 100000) / 100000 for _ in range(100)]
    #white_noise = [math.sin(2 * pi * 1000 * t / s) for t in range(48000)]
    filtered_noise = bandpass_filter(500, 200, white_noise)
    print(filtered_noise)

if (__name__ == '__main__'):
    run()
