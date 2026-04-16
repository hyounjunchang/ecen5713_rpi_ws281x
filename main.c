#include "ws2812b_wrapper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "tiles_grid.h"


#define LANE_COUNT 4

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

typedef struct {
    uint8_t lane[LANE_COUNT];
} frame_t;

typedef struct {
    frame_t *frames;
    size_t count;
    size_t capacity;
} frame_buffer_t;

static void frame_buffer_init(frame_buffer_t *buf)
{
    buf->frames = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

static void frame_buffer_free(frame_buffer_t *buf)
{
    free(buf->frames);
    buf->frames = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

static int frame_buffer_push(frame_buffer_t *buf, frame_t frame)
{
    if (buf->count == buf->capacity) {
        size_t new_capacity = (buf->capacity == 0) ? 128 : buf->capacity * 2;
        frame_t *new_frames = realloc(buf->frames, new_capacity * sizeof(frame_t));
        if (!new_frames) {
            return -1;
        }
        buf->frames = new_frames;
        buf->capacity = new_capacity;
    }
    buf->frames[buf->count++] = frame;
    return 0;
}

static int parse_csv_frames(const char *filename, frame_buffer_t *out)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char line[256];

    /* Read and skip header */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        fprintf(stderr, "Error: empty file\n");
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        frame_t frame;
        int l0, l1, l2, l3;

        /* Remove trailing newline if present */
        line[strcspn(line, "\r\n")] = '\0';

        /* Skip empty lines */
        if (line[0] == '\0') {
            continue;
        }

        if (sscanf(line, "%d,%d,%d,%d", &l0, &l1, &l2, &l3) != 4) {
            fprintf(stderr, "Warning: skipping malformed line: %s\n", line);
            continue;
        }

        frame.lane[0] = (uint8_t)l0;
        frame.lane[1] = (uint8_t)l1;
        frame.lane[2] = (uint8_t)l2;
        frame.lane[3] = (uint8_t)l3;

        if (frame_buffer_push(out, frame) != 0) {
            fclose(fp);
            fprintf(stderr, "Error: out of memory while storing frames\n");
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

int main() {
    // setting up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (init_led_grid() != WS2811_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Build a blank row (black) for spacing between color rows
    ws2811_led_t blank_row[WIDTH];
    memset(blank_row, 0, sizeof(blank_row));

    // Build the colored row — one solid color across the full width
    ws2811_led_t color_row[4][WIDTH];
    memset(color_row, 0, sizeof(color_row));
    int color_index = 0;
    int lane_index = 0;
    int tick = 0;
    const int block_height = 3;   // rows of color per block
    const int gap_height   = 3;   // blank rows between blocks
    int period       = block_height + gap_height;

    
    frame_buffer_t frames;
    frame_buffer_init(&frames);

    if (parse_csv_frames("LetitBe.csv", &frames) != 0) {
        fprintf(stderr, "Failed to parse CSV\n");
        return 1;
    }
    

   
    ws2811_led_t active_color = dotcolors[0];
    
    for (int y = 0 ; y < 4 ; y++)
    {
        for (int x = 0; x < WIDTH_BLOCK; x++) {
        	color_row[y][x] = dotcolors[y];
    	}
    }

    

    for (size_t i = 0; running && i < frames.count; i++) {
	  for (int lane = 0; lane < 4; lane++) {
		if (frames.frames[i].lane[lane]) {
		    grid_insert_lane(color_row[lane], lane);
		} else {
		    grid_insert_lane(blank_row, lane);
		}
	    }

	    if (render_led_grid() != 0) {
		break;
	    }

	    usleep(100 * 1000);   // if each CSV row is 10 ms
    }

    clear_led_grid(); // turn off all led strip lights
    usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec


    free_led_grid(); // free memory and buses

    return EXIT_SUCCESS;
}
