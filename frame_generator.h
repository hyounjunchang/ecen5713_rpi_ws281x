#ifndef FRAME_GENERATOR_H
#define FRAME_GENERATOR_H

#include "frame_parser.h"
#include "ws2812b_wrapper.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
void init_frame();
void render_frame(bool active_row , uint8_t lane);
void frame_buffer_init(frame_buffer_t *buf);
void frame_buffer_free(frame_buffer_t *buf);
int frame_buffer_push(frame_buffer_t *buf, frame_t frame);
int parse_csv_frames(const char *filename, frame_buffer_t *out);

#endif

