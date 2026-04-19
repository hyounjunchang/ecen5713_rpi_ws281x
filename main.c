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
    
    init_frame();


    for (size_t i = 0; running; i++) {

	render_frame(1,0);	
	usleep(100 * 1000);   // if each CSV row is 10 ms
    }

    clear_led_grid(); // turn off all led strip lights
    usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec


    free_led_grid(); // free memory and buses

    return EXIT_SUCCESS;
}
