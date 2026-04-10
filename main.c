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

    while (running && tick < 100) {
        // Determine what to insert at the top this tick
        if (tick % period < block_height) {
            // Colored portion of the block
            ws2811_led_t color = dotcolors[color_index];
            for (int x = 0; x < WIDTH; x++) {
                color_row[x] = color;
            }
            matrix_insert_top_row(color_row);
        } else {
            // Gap between blocks
            matrix_insert_top_row(blank_row);
        }

        // Advance color every time we finish a full block+gap cycle
        if (tick % period == period - 1) {
            color_index = (color_index + 1) % COLOR_COUNT;
        }

        tick++;

        if (render_led_grid() != 0) {
            break;
        }

        usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec
    }

    clear_led_grid();
    render_led_grid();
    free_led_grid();

    return EXIT_SUCCESS;
}