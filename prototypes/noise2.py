import math
import random
import shlex
import struct
import subprocess
import tempfile
import traceback
from functools import reduce

SAMPLE_RATE = 11025
DURATION_SECONDS = 1
NOISE_CENTER_FREQ = 500
NOISE_BANDWIDTH = 200
REPETITIONS = 1

def write_samples(p):
    # create an array of SAMPLE_RATE oscillators
    nb_samples = SAMPLE_RATE * DURATION_SECONDS
    initial_freq = NOISE_CENTER_FREQ - (NOISE_BANDWIDTH / 2)
    final_freq = initial_freq + NOISE_BANDWIDTH
    oscillators = []
    for x in range(SAMPLE_RATE):
        rnd = random.random()
        o = {
            'freq': (x/SAMPLE_RATE * NOISE_BANDWIDTH) + initial_freq,
            'osc': complex(math.cos(rnd), math.sin(rnd))
        }
        oscillators.append(o)

    # rotate them by their respective frequencies
    # get their real parts
    # push them to the samples array
    # iterate for DURATION_SECONDS times
    print('Starting writing frames...')
    for x in range(SAMPLE_RATE): #SAMPLE_RATE
        s = reduce(lambda x, y: x + y['osc'].real, oscillators, 0) / SAMPLE_RATE
        print(f'\rwriting frame {x}...', end='')
        p.stdin.write(struct.pack('f', s))
        for o in oscillators:
            o['osc'] *= math.e ** (o['freq'] * (2 * math.pi / SAMPLE_RATE) * 1j)
    print('')
    p.communicate()

def run():
    outfilename = "/tmp/noise.mp3"
    ffmpeg_command = f"ffmpeg -v 0 -y -f f32le -ar {SAMPLE_RATE} -ac 1 -i - {outfilename}"
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


