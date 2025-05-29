import shlex
import subprocess
import tempfile
import traceback

DURATION_SECONDS = 5
FRAME_RATE = 25
HEIGHT = 240
WIDTH = 320

def write_frames(p):
    total_frames = FRAME_RATE * DURATION_SECONDS
    total_pixels = HEIGHT * WIDTH
    frame_color = (0x00, 0x00, 0x00)
    color_increment = 0xFF // total_frames
    for i in range(1, total_frames + 1):
        print("Writing frame")
        p.stdin.write(bytearray(frame_color * total_pixels))
        frame_color = (frame_color[0] + color_increment,
                       frame_color[1] + (color_increment // 2), 0x00)
    p.communicate()

def run():
    outfile = tempfile.NamedTemporaryFile(delete=False)
    outfile.close()
    outfilename = f"{outfile.name}.mp4"
    ffmpeg_command = (
        f'ffmpeg -f rawvideo -pixel_format rgb24 -video_size 320x240 -r {FRAME_RATE} '
        f'-i pipe: -c:v libx264 -pix_fmt yuv420p {outfilename}'
    )
    args = shlex.split(ffmpeg_command)
    with subprocess.Popen(args, stdin=subprocess.PIPE) as p:
        try:
            write_frames(p)
            print(f"Wrote video to file {outfilename}")
        except Exception as e:
            print("Error writing video sample to output file:", e)
            traceback.print_exc()
        try:
            p.wait(timeout=5)
            print("ffmpeg process has finished")
        except subprocess.TimeoutExpired:
            p.kill()
            print("ffmpeg process was killed")

if __name__ == '__main__':
    run()

