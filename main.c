#include "ws2812b_wrapper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

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

    int color_index = 0;
    int tick = 0;
    const int block_height = 1;   // rows of color per block
    const int gap_height   = 3;   // blank rows between blocks
    const int period       = block_height + gap_height;


    // Example 1: color row falling down to bottom
    while (running && tick < 80) {
        // Determine what to insert at the top this tick
        if (tick % period < block_height) {
            // Colored portion of the block
            ws2811_led_t color = dotcolors[color_index];
            for (int x = 0; x < WIDTH; x++) {
                color_row[x] = color;
            }
            grid_insert_top_row(color_row);
        } else {
            // Gap between blocks
            grid_insert_top_row(blank_row);
        }

        // Advance color every time we finish a full block+gap cycle
        if (tick % period == period - 1) {
            color_index = (color_index + 1) % COLOR_COUNT;
        }

        tick++;

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