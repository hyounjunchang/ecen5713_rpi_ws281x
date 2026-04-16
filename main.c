#include "ws2812b_wrapper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "tiles_grid.h"
#include "frame_parser.h"
#include "frame_generator.h"

#define LANE_COUNT 4

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
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
