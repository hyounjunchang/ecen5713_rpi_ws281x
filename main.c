#include "ws2812b_wrapper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "tiles_grid.h"
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
    ws2811_led_t color_row[WIDTH];
    memset(color_row, 0, sizeof(color_row));
    int color_index = 0;
    int lane_index = 0;
    int tick = 0;
    const int block_height = 3;   // rows of color per block
    const int gap_height   = 3;   // blank rows between blocks
    int period       = block_height + gap_height;

    
    note_t lane1[3];
    
    lane1[0].duration_ms = 200;
    lane1[0].time_ms = 1000;
    lane1[1].duration_ms = 100;
    lane1[1].time_ms = 2000;

    lane1[2].duration_ms = 100;
    lane1[2].time_ms = 3000;
    
    uint8_t lane1_index = 0;
    int shaded_duration = lane1[0].time_ms;
    int darkened_duration = lane1[0].duration_ms + lane1[0].time_ms - lane1[1].duration_ms;
    period = shaded_duration + darkened_duration;
    // Example 1: color row falling down to bottom
    while (running && tick < 800) {
        // Determine what to insert at the top this tick
        if (!(tick % period))
        {
         
         shaded_duration = lane1[lane1_index].time_ms;
         
         if(lane1_index + 1 < sizeof(lane1)/sizeof(note_t))
         {
                  darkened_duration = lane1[lane1_index+1].time_ms - lane1[lane1_index].duration_ms + lane1[lane1_index].time_ms;			     	 
         }
         color_index = (color_index + 1) % COLOR_COUNT;
         period = shaded_duration + darkened_duration;
         lane1_index += 1;

        }
        if (tick % period < shaded_duration) {
            // Colored portion of the block
            ws2811_led_t color = dotcolors[color_index];
            for (int x = 0; x < WIDTH_BLOCK; x++) {
                color_row[x] = color;
            }
            grid_insert_lane(color_row , lane_index) ;
            grid_insert_lane(blank_row, (lane_index + 1) & 3);
            grid_insert_lane(blank_row, (lane_index + 2) & 3);
            grid_insert_lane(blank_row, (lane_index + 3) & 3);
        } else {
            // Gap between blocks
            grid_insert_lane(blank_row , lane_index) ;
            grid_insert_lane(blank_row, (lane_index + 1) & 3);
            grid_insert_lane(blank_row, (lane_index + 2) & 3);
            grid_insert_lane(blank_row, (lane_index + 3) & 3);
        }

        // Advance color every time we finish a full block+gap cycle

        tick+=10;

        if (render_led_grid() != 0) {
            break; // LED grid not rendered
        }

        usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec
    }

    clear_led_grid(); // turn off all led strip lights
    usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec

    // Example 2: Diagonal Line (Bottom-Left to Top Right) Shfting down
    // Only 4 LEDs are lit every row, shifting towards right each time, with colors changing on each row
    while (running && tick < 200) {
        // Determine what to insert at the top this tick
        int start_x = tick % WIDTH;
        int end_x = (tick+4) % WIDTH;

        ws2811_led_t color = dotcolors[tick % COLOR_COUNT];
        for (int x = 0; x < WIDTH; x++){
            color_row[x] = 0;
        }
        for (int x = start_x; x != end_x; x = (x+1) % WIDTH){
            color_row[x] = color;
        }

        grid_insert_top_row(color_row);
        if (render_led_grid() != 0) {
            break; // LED grid not rendered
        }

        tick++;
        usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec
    }

    clear_led_grid(); // turn off all led strip lights
    free_led_grid(); // free memory and buses

    return EXIT_SUCCESS;
}
