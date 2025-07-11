#include <assert.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parser.h"
#include "config.h"
#include "token.h"
#include "video.h"
#include "util.h"

typedef struct frame_data_s {
  const char *glyph_text;
  unsigned char *frame_buffer;
  struct frame_data_s *next;
} frame_data_t;

static frame_data_t *frames_cache = NULL;

static void getFrame(const config_t *config, const char *glyph_text,
    unsigned char **frame_buffer) {
  frame_data_t *tmp, *new_frame, *last_frame = NULL;
  int frame_size;

  tmp = frames_cache;
  while (tmp != NULL) {
    if (strcmp(tmp->glyph_text, glyph_text) == 0) {
      *frame_buffer = tmp->frame_buffer;
      return;
    }
    last_frame = tmp;
    tmp = tmp->next;
  }

  frame_size = config->video_height * config->video_width * sizeof(unsigned char) * 3;

  // the cairo-related code below was generated by AI (ChatGPT) using the
  // following prompt:
  //
  // I will ask a code snippet using the Cairo library, in C. I want to
  // generate a rectangular canvas. The background is black. The canvas's
  // aspect ratio is 16:9. In the middle of the rectangle, there's gonna be a
  // letter A, in white. At the end of the snippet, I want to obtain an array
  // of bytes representing each pixel of that image. Each pixel should be
  // represented by 3 bytes, representing the RGB components

  // Create image surface and context
  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
      config->video_width, config->video_height);
  cairo_t *cr = cairo_create(surface);

  // Fill background with black
  cairo_set_source_rgb(cr, 0, 0, 0); // Black
  cairo_paint(cr);

  // Draw white letter "A" in the center
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, config->video_height * 0.5); // Scale font to 50% of height

  cairo_text_extents_t extents;
  cairo_text_extents(cr, glyph_text, &extents);

  double x = (config->video_width - extents.width) / 2 - extents.x_bearing;
  double y = (config->video_height - extents.height) / 2 - extents.y_bearing;

  cairo_move_to(cr, x, y);
  cairo_set_source_rgb(cr, 1, 1, 1); // White
  cairo_show_text(cr, glyph_text);

  cairo_font_extents_t font_extents;
  cairo_font_extents(cr, &font_extents);

  // Draw overscore bar if text has more than one character
  if (strlen(glyph_text) > 1) {
    // Position the bar slightly above the text
    double bar_x = x + extents.x_bearing;
    double bar_y = y - font_extents.ascent * 0.9;
    double bar_width = extents.width + extents.x_bearing;
    double bar_thickness = extents.height * 0.05; // 5% of text height

    cairo_set_line_width(cr, bar_thickness);
    cairo_move_to(cr, bar_x, bar_y);
    cairo_line_to(cr, x + bar_width, bar_y);
    cairo_stroke(cr);
  }

  cairo_surface_flush(surface);

  // Extract raw pixel data (RGB)
  unsigned char *data = cairo_image_surface_get_data(surface);
  int stride = cairo_image_surface_get_stride(surface);
  unsigned char *rgb_data = malloc(frame_size); // RGB (3 bytes per pixel)

  for (int y = 0; y < config->video_height; y++) {
    unsigned char *src_row = data + y * stride;
    for (int x = 0; x < config->video_width; x++) {
      unsigned char *pixel = src_row + x * 4; // CAIRO_FORMAT_RGB24 uses
                                              // 4 bytes (ARGB 32-bit)
      int i = (y * config->video_width + x) * 3;
      rgb_data[i + 0] = pixel[2]; // Red
      rgb_data[i + 1] = pixel[1]; // Green
      rgb_data[i + 2] = pixel[0]; // Blue
    }
  }

  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  new_frame = (frame_data_t *) fmalloc(sizeof(frame_data_t));
  new_frame->glyph_text = strdup(glyph_text);
  new_frame->next = NULL;
  new_frame->frame_buffer = rgb_data;

  if (last_frame == NULL) {
    last_frame = new_frame;
  } else {
    last_frame->next = new_frame;
  }

  *frame_buffer = new_frame->frame_buffer;
}

static void writeGlyphText(const char *glyph_text, FILE *pipeout,
    const config_t *config, int duration_ms, float *frames_mismatch) {
  unsigned char *framebuffer = NULL;
  float nb_frames_ideal = (duration_ms / 1000.) * config->framerate;
  float int_mismatch = 0.;
  int nb_frames_real = (int) nb_frames_ideal;
  *frames_mismatch += nb_frames_real - nb_frames_ideal;
  modff(*frames_mismatch, &int_mismatch);
  int frames_adj = (abs(int_mismatch) < nb_frames_real) ? -int_mismatch : -nb_frames_real + 1;
  nb_frames_real += frames_adj;
  *frames_mismatch += frames_adj;
  // TODO this is being computed in other parts of this module. We could compute this only
  // once
  int bufsz = config->video_height * config->video_width
    * sizeof(unsigned char) * 3;

  getFrame(config, glyph_text, &framebuffer);
  if (framebuffer == NULL) {
    fprintf(stderr, "Could not obtain the frame data for glyph text (%s)\n", glyph_text);
    exit(1);
  }

  for (int i = 0; i < nb_frames_real; ++i) {
    size_t written = fwrite(framebuffer, bufsz, 1, pipeout);
    if (written < 1) {
      fprintf(stderr, "Error send video frames to ffmpeg\n");
      exit(1);
    }
  }
}

static void uninitializeVideo() {
  // free frames_cache nodes and their respective attributes, especially the
  // frame buffers
}

void writeVideo(const config_t *config, const token_bag_t *token_bag,
    const char *video_filename) {

  FILE *pipeout = NULL;
  const char cmd_fmt[] = "ffmpeg -v 0 -y -f rawvideo -pixel_format rgb24 "
        "-video_size 640x480 -r %d "
        "-i pipe: -c:v libx264 -pix_fmt yuv420p -f mp4 %s";
  int cmd_bufsz;
  char *ffmpeg_cmd;
  // tokens/glyphs
  token_t *token = NULL;
  glyph_t *glyph = NULL;
  float frames_mismatch = 0.;

  if (!is_valid_filename(video_filename)) {
    fprintf(stderr, "Invalid video file name\n");
    exit(1);
  }

  cmd_bufsz = snprintf(NULL, 0, cmd_fmt, config->framerate, video_filename);
  cmd_bufsz++;
  ffmpeg_cmd = (char *) fmalloc(cmd_bufsz + 1);
  snprintf(ffmpeg_cmd, cmd_bufsz, cmd_fmt, config->framerate, video_filename);

  // TODO consider changing to `run_and_capture`, in `util.h`
  pipeout = popen(ffmpeg_cmd, "w");
  if (pipeout == NULL) {
    fprintf(stderr, "Could not open video file\n");
    exit(1);
  }

  token = token_bag->token_head;

  while (token != NULL) {
    glyph = token->glyph_head;
    const char *stored_glyph_text = NULL;

    while (glyph != NULL) {
      const char *morse = glyph->morse;
      int morse_len = strlen(morse);
      int duration_units = 0;
      int farns_units = 0;

      for (int i = 0; i < morse_len; ++i) {
        duration_units += morse[i] == '.' ? 1 : 3;
        int is_last = i == morse_len - 1;
        if (!is_last) duration_units += 1;
      }
      //duration_units += morse_len - 1; // spacers between each dih and dah

      writeGlyphText((config->morse_mode == MODE_SYNC) ? glyph->text : " ", pipeout, config,
          duration_units * config->normal_unit_ms, &frames_mismatch);
      if (config->morse_mode == MODE_GUESS) {
        stored_glyph_text = glyph->text;
      }

      glyph = glyph->next;

      if (glyph == NULL) {
        // the word is finished. Output inter-word spacer. (Farnsworth will equal normal
        // durations when no farnsworth duration is specified
        farns_units = 7;
      } else {
        // there are still characters in the current word. Output the inter-character
        // spacer
        farns_units = 3;
      }


      if (config->morse_mode == MODE_SYNC) {
        writeGlyphText(" ", pipeout, config, config->farnsworth_unit_ms * farns_units,
            &frames_mismatch);
      } else {
        int glyph_duration_ms = config->normal_unit_ms;
        int blank_duration_ms = config->farnsworth_unit_ms * farns_units - glyph_duration_ms;
        assert(stored_glyph_text != NULL);
        writeGlyphText(stored_glyph_text, pipeout, config, glyph_duration_ms, &frames_mismatch);
        writeGlyphText(" ", pipeout, config, blank_duration_ms, &frames_mismatch);
      }
    }

    token = token->next;
  }

  fclose(pipeout);
}
