#ifndef FRAME_GENERATOR_H
#define FRAME_GENERATOR_H

#include "frame_parser.h"
void frame_buffer_init(frame_buffer_t *buf);
void frame_buffer_free(frame_buffer_t *buf);
int frame_buffer_push(frame_buffer_t *buf, frame_t frame);
int parse_csv_frames(const char *filename, frame_buffer_t *out);

#endif

