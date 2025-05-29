import math
import shlex
import struct
import subprocess
import tempfile
import traceback

SAMPLE_RATE = 22050
DURATION_SECONDS = 2
FREQ_HZ = 400

def write_samples(p):
    oscillator = 0 + 1j
    for _ in range(0, SAMPLE_RATE * DURATION_SECONDS):
        #p.communicate(input=struct.pack('f', oscillator.real))
        p.stdin.write(struct.pack('f', oscillator.real))
        oscillator *= math.e ** (FREQ_HZ * (2 * math.pi / SAMPLE_RATE) * 1j)
    p.communicate()

def run():
    outfile = tempfile.NamedTemporaryFile(delete=False)
    outfile.close()
    outfilename = f"{outfile.name}.mp3"
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

