# Generated with ChatGPT. Prompts are following:
#
# this is a python project. The goal is to have an array of floats (32 bits) representing
# audio noise in PCM format. The resulting noise should sound like white noise after a
# pass-band filter. The low and high frequencies of such filter are configurable, and will
# be received as parameters by the function generating this noise. Generate code for that
# function
#
# #Write a function with same functionality WITHOUT using NumPy
#
# #write a function with the same functionality using Python's standard library
# exclusively

import random
import math
from array import array
from typing import List
import shlex
import subprocess
import traceback
import struct

SAMPLE_RATE = 22050
DURATION_SECONDS = 5

def biquad_bandpass_coeffs(sample_rate: int, low_freq: float, high_freq: float, q: float = 0.707) -> List[List[float]]:
    """Create band-pass filter coefficients using cascaded biquad sections."""
    # Design a single band-pass filter centered between low and high frequency
    center_freq = (low_freq + high_freq) / 2
    bandwidth = high_freq - low_freq
    omega = 2 * math.pi * center_freq / sample_rate
    alpha = math.sin(omega) * math.sinh(math.log(2) / 2 * bandwidth * omega / math.sin(omega))
    b0 = alpha
    b1 = 0.0
    b2 = -alpha
    a0 = 1 + alpha
    a1 = -2 * math.cos(omega)
    a2 = 1 - alpha
    # Normalize coefficients
    return [
        [b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0]
    ]

def apply_biquad_filter(samples: List[float], coeffs: List[List[float]]) -> List[float]:
    """Apply biquad filter(s) to a signal."""
    for b0, b1, b2, a1, a2 in coeffs:
        x1 = x2 = y1 = y2 = 0.0
        output = []
        for x0 in samples:
            y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2
            output.append(y0)
            x2, x1 = x1, x0
            y2, y1 = y1, y0
        samples = output
    return samples

def generate_filtered_noise(duration_sec: float, sample_rate: int, low_freq: float, high_freq: float) -> array:
    """
    Generate band-passed white noise using Python's standard library only.

    Returns:
        array: Array of float32 audio samples.
    """
    num_samples = int(duration_sec * sample_rate)
    # Generate white noise (mean 0, stddev 1)
    noise = [random.gauss(0, 1) for _ in range(num_samples)]
    # Get band-pass filter coefficients
    coeffs = biquad_bandpass_coeffs(sample_rate, low_freq, high_freq)
    # Apply filter
    filtered = apply_biquad_filter(noise, coeffs)
    # Convert to float32 array
    return array('f', filtered)

def write_samples(p):
    #samples = band_limited_noise(300, 700, samples=SAMPLE_RATE * DURATION_SECONDS,
    #                             samplerate=SAMPLE_RATE)
    samples = generate_filtered_noise(duration_sec=DURATION_SECONDS,
                                       sample_rate=SAMPLE_RATE,
                                       low_freq=400, high_freq=600)
    #multiplier = 0.5
    samples = [s * 0.3 for s in samples]
    for s in samples:
        p.stdin.write(struct.pack('f', s))
    p.communicate()

def run():
    outfilename = "/tmp/noise.mp3"
    ffmpeg_command = f"ffmpeg -y -f f32le -ar {SAMPLE_RATE} -ac 1 -i - {outfilename}"
    args = shlex.split(ffmpeg_command)
    with subprocess.Popen(args, stdin=subprocess.PIPE) as p:
        try:
            write_samples(p)
            print(f"Wrote audio to file {outfilename}")
        except Exception as e:
            print("Error writing audio frames to output file:", e)
            traceback.print_exc()
        try:
            p.wait(timeout=5)
            print("ffmpeg process has finished")
        except subprocess.TimeoutExpired:
            p.kill()
            print("ffmpeg process was killed")

if __name__ == '__main__':
    run()

