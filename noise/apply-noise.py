try:
    import numpy as np
except ImportError:
    print("Error: numpy is required to run this script. Please install it with 'pip install numpy'.")
    exit(1)

import sys
import os
import shutil
import struct
import subprocess
import tempfile

if len(sys.argv) != 5:
    print(f"Usage: {sys.argv[0]} <media_file> <center_frequency> <bandwidth> <level>")
    exit(1)

media_file = sys.argv[1]

# Parse and validate center_frequency
try:
    center_frequency = int(sys.argv[2])
except ValueError:
    print("Error: center_frequency must be an integer.")
    exit(1)

# Parse and validate bandwidth
try:
    bandwidth = int(sys.argv[3])
except ValueError:
    print("Error: bandwidth must be an integer.")
    exit(1)

# Parse level (string, no validation for now)
level = sys.argv[4]

# Check that the file exists
if not os.path.isfile(media_file):
    print(f"Error: File '{media_file}' does not exist.")
    exit(1)

# Check that the file has a supported media extension
SUPPORTED_EXTENSIONS = {'.mp3', '.mp4', '.wav', '.m4a', '.mov'}
_, ext = os.path.splitext(media_file)
if ext.lower() not in SUPPORTED_EXTENSIONS:
    print(f"Error: File extension '{ext}' is not a supported media type. Supported types: {', '.join(SUPPORTED_EXTENSIONS)}")
    exit(1)

# parameter: audio or video file containing morse code audio

# algorithm:

# validate media file

# validate ffprobe and ffmpeg

for cmd in ['ffmpeg', 'ffprobe']:
    if shutil.which(cmd) is None:
        print(f"Error: '{cmd}' is not available in your PATH. Please install it to proceed.")
        exit(1)

def get_media_duration(filename):
    try:
        result = subprocess.run(
            [
                'ffprobe', '-v', 'error',
                '-show_entries', 'format=duration',
                '-of', 'default=noprint_wrappers=1:nokey=1',
                filename
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=True
        )
        duration_str = result.stdout.strip()
        duration = float(duration_str)
        return duration
    except Exception as e:
        print(f"Error: Could not retrieve duration for '{filename}'. ffprobe output: {result.stderr.strip()}")
        exit(1)

media_duration = get_media_duration(media_file)
# print(f"Media duration: {media_duration} seconds")

# generate a noise file of that same duration (use numpy's fft facilities)
# src: https://stackoverflow.com/questions/33933842/how-to-generate-noise-in-frequency-range-with-numpy
def fftnoise(f):
    f = np.array(f, dtype='complex')
    Np = (len(f) - 1) // 2
    phases = np.random.rand(Np) * 2 * np.pi
    phases = np.cos(phases) + 1j * np.sin(phases)
    f[1:Np+1] *= phases
    f[-1:-1-Np:-1] = np.conj(f[1:Np+1])
    o = np.fft.ifft(f)
    return o.real

# src: https://stackoverflow.com/questions/33933842/how-to-generate-noise-in-frequency-range-with-numpy
def band_limited_noise(min_freq, max_freq, samples=1024, samplerate=1):
    freqs = np.abs(np.fft.fftfreq(samples, 1/samplerate))
    f = np.zeros(samples)
    idx = np.where(np.logical_and(freqs>=min_freq, freqs<=max_freq))[0]
    f[idx] = 1
    return fftnoise(f)

def generate_noise_file(duration, min_freq, max_freq):
    noise = band_limited_noise(min_freq, max_freq, int(duration * 44100), 44100)
    return noise

min_freq = center_frequency - bandwidth // 2
max_freq = min_freq + bandwidth

noise_samples = generate_noise_file(media_duration, min_freq, max_freq)
# noise_samples is a numpy array of samples

# Write noise_samples to a temporary mp3 file using ffmpeg
with tempfile.NamedTemporaryFile(suffix='.mp3', delete=False) as tmp_mp3:
    noise_mp3_filename = tmp_mp3.name

ffmpeg_cmd = [
    'ffmpeg', '-y',
    '-f', 'f32le', '-ar', '44100', '-ac', '1', '-i', '-',
    '-f', 'mp3', noise_mp3_filename
]

process = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE)
try:
    noise_samples *= 1000
    for s in noise_samples:
        process.stdin.write(struct.pack('f', s))
    process.communicate()
except Exception as e:
    print(f"Error writing noise samples to ffmpeg: {e}")
    process.kill()
    exit(1)

# noise_mp3_filename now contains the path to the generated noise mp3 file

# Mix the input media file with the generated noise mp3 file
with tempfile.NamedTemporaryFile(suffix=os.path.splitext(media_file)[1], delete=False) as tmp_merged:
    merged_filename = tmp_merged.name

is_audio = os.path.splitext(media_file)[1].lower() in {'.mp3', '.wav', '.m4a'}

if is_audio:
    # Audio-only: mix with amix
    ffmpeg_mix_cmd = [
        'ffmpeg', '-y',
        '-i', media_file,
        '-i', noise_mp3_filename,
        '-filter_complex', 'amix=inputs=2:duration=first:dropout_transition=2',
        '-c:a', 'mp3',
        merged_filename
    ]
else:
    # Video: mix audio streams, keep video from original
    ffmpeg_mix_cmd = [
        'ffmpeg', '-y',
        '-i', media_file,
        '-i', noise_mp3_filename,
        '-filter_complex', '[0:a][1:a]amix=inputs=2:duration=first:dropout_transition=2[aout]',
        '-map', '0:v?',
        '-map', '[aout]',
        '-c:v', 'copy',
        '-c:a', 'mp3',
        merged_filename
    ]

mix_result = subprocess.run(ffmpeg_mix_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
if mix_result.returncode != 0:
    print(f"Error: ffmpeg failed to mix files.\n{mix_result.stderr.decode()}")
    exit(1)

# Replace the original media file with the merged file
try:
    os.replace(merged_filename, media_file)
except Exception as e:
    print(f"Error replacing original media file: {e}")
    exit(1)

# Success
print(f"Successfully mixed noise into {media_file}")

