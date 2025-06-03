import math
import random
import shlex
import struct
import subprocess
import tempfile
import traceback
import numpy as np

SAMPLE_RATE = 22050
DURATION_SECONDS = 5
NOISE_AVG = 500
NOISE_STD_DEV = 8

def gaussian(freq):
    mi = NOISE_AVG
    sigma = NOISE_STD_DEV
    g = (
        (
            1. / (sigma * math.sqrt(2 * math.pi))
        )
        *
        (
            math.e ** (-0.5 * ((freq - mi) / sigma) ** 2)
        )
    )
    if g < 0: g = 0
    return g

# src: https://stackoverflow.com/questions/33933842/how-to-generate-noise-in-frequency-range-with-numpy
def fftnoise(f):
    f = np.array(f, dtype='complex')
    Np = (len(f) - 1) // 2
    phases = np.random.rand(Np) * 2 * np.pi
    phases = np.cos(phases) + 1j * np.sin(phases)
    f[1:Np+1] *= phases
    f[-1:-1-Np:-1] = np.conj(f[1:Np+1])
    return np.fft.ifft(f).real

# src: https://stackoverflow.com/questions/33933842/how-to-generate-noise-in-frequency-range-with-numpy
def band_limited_noise(min_freq, max_freq, samples=1024, samplerate=1):
    freqs = np.abs(np.fft.fftfreq(samples, 1/samplerate))
    f = np.zeros(samples)
    idx = np.where(np.logical_and(freqs>=min_freq, freqs<=max_freq))[0]
    f[idx] = 1
    return fftnoise(f)

def write_samples0(p):
    oscillator = 0 + 1j
    for _ in range(0, SAMPLE_RATE * DURATION_SECONDS):
        #p.communicate(input=struct.pack('f', oscillator.real))
        freq = random.randint(200, 800)
        factor = (freq * (2 * math.pi / SAMPLE_RATE) * 1j * gaussian(freq) * 752)
        oscillator *= (
            (math.e ** factor)
        )
        #multiplier = gaussian(freq) * 300 # for normalization
        multiplier = 0.8
        p.stdin.write(struct.pack('f', oscillator.real * multiplier))
    p.communicate()

def write_samples(p):
    samples = band_limited_noise(300, 700, samples=SAMPLE_RATE * DURATION_SECONDS,
                                 samplerate=SAMPLE_RATE)
    multiplier = 0.5
    samples *= 326 * multiplier
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

