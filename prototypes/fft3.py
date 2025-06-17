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

def write_file(samples, filename):
    with open(filename, 'w') as f:
        for s in samples:
            f.write("{:10.5f},{:10.5f}\n".format(s.real, s.imag))

# src: https://stackoverflow.com/questions/33933842/how-to-generate-noise-in-frequency-range-with-numpy
def fftnoise(f):
    f = np.array(f, dtype='complex')
    Np = (len(f) - 1) // 2
    phases = np.random.rand(Np) * 2 * np.pi
    #phases = np.repeat(np.array([1.25 * np.pi]), Np)
    phases = np.cos(phases) + 1j * np.sin(phases)
    f[1:Np+1] *= phases
    f[-1:-1-Np:-1] = np.conj(f[1:Np+1])
    write_file(f, "/tmp/noise1-input.csv")
    o = np.fft.ifft(f)
    write_file(o, "/tmp/noise1-output.csv")
    r = np.fft.fft(o)
    write_file(r, "/tmp/noise1-reinput.csv")
    return o.real

# src: https://stackoverflow.com/questions/33933842/how-to-generate-noise-in-frequency-range-with-numpy
def band_limited_noise(min_freq, max_freq, samples=1024, samplerate=1):
    freqs = np.abs(np.fft.fftfreq(samples, 1/samplerate))
    f = np.zeros(samples)
    idx = np.where(np.logical_and(freqs>=min_freq, freqs<=max_freq))[0]
    f[idx] = 1
    return fftnoise(f)

def write_samples(p):
    samples = band_limited_noise(475, 525, samples=SAMPLE_RATE * DURATION_SECONDS,
                                 samplerate=SAMPLE_RATE)
    samples /= np.max(samples)
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


