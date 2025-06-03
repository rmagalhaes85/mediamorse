#ifndef VIDEO_H
#define VIDEO_H

#include "config.h"
#include "parser.h"
#include "token.h"

void writeVideo(const config_t *config, const token_bag_t *token_bag,
    const char *video_filename);

#endif
