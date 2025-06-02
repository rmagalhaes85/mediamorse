#ifndef AUDIO_H
#define AUDIO_H

#include "config.h"
#include "parser.h"

void writeAudio(const config_t *config, const token_bag_t *token_bag,
    const char *audio_filename);

#endif
