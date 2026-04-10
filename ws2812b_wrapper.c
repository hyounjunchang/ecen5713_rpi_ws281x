/*
Refactored code from main.c example from ws281x library
Claude Code was used to help debug: https://claude.ai/chat/09145b22-92ff-4de6-bf07-5244e09bf8ab
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"

#include "ws2812b_wrapper.h"

const int width = WIDTH;
const int height = HEIGHT;

ws2811_led_t dotcolors[COLOR_COUNT] =
{
    0x00200000,  // red
    0x00201000,  // orange
    0x00202000,  // yellow
    0x00002000,  // green
    0x00002020,  // lightblue
    0x00000020,  // blue
    0x00100010,  // purple
    0x00200010,  // pink
};

// Example ledstring initialization
ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .invert = 0,
            .count = LED_COUNT,
            .strip_type = STRIP_TYPE,
            .brightness = 255,
        },
        [1] =
        {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};

ws2811_led_t *matrix;


int init_led_grid()
{
    ws2811_return_t ret;
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    matrix = malloc(sizeof(ws2811_led_t) * LED_COUNT);
    if (!matrix) {
        ws2811_fini(&ledstring);  // clean up on malloc failure
        return -1;
    }

    memset(matrix, 0, sizeof(ws2811_led_t) * LED_COUNT);
    return WS2811_SUCCESS;
}

void free_led_grid()
{
    free(matrix);
    matrix = NULL;
    ws2811_fini(&ledstring);
}

int render_led_grid()
{
    if (!matrix) return -1;

    ws2811_return_t ret;
    int retval = 0;
    matrix_reorder_to_pcb();
    matrix_render();
    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        retval = -1;
    }

    matrix_reorder_to_pcb();  // reverse: applying same swap twice restores original
    return retval;
}

int clear_led_grid(){
    ws2811_return_t ret;
    int retval = 0;
    matrix_clear();
    matrix_render();
    if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
        retval = -1;
    }
    return retval;
}

void matrix_insert_top_row(ws2811_led_t* colors){
    int x, y;
    // shift led down by one row
    for (y = 0; y < height - 1; y++){
        for (x = 0; x < width; x++){
            matrix[y * width + x] = matrix[(y + 1)*width + x];
        }
    }
    for (x = 0; x < width; x++){
        matrix[(height-1)*width + x] = colors[x];
    }

}

void matrix_render()
{
    int x, y;

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            ledstring.channel[0].leds[(y * width) + x] = matrix[y * width + x];
        }
    }
}

// re-orders matrix same as pcb
void matrix_reorder_to_pcb()
{
    int x, y;

    for (y = 0; y < height; y++)
    {
        // odd row only
        if (y%2){
            for (x = 0; x < width/2; x++)
            {
                ws2811_led_t temp = matrix[y * width + x];
                matrix[y * width + x] = matrix[y * width + (width - x - 1)];
                matrix[y * width + (width - x - 1)] = temp;
            } 
        }

    }
}

void matrix_clear()
{
    int x, y;

    for (y = 0; y < (height ); y++)
    {
        for (x = 0; x < width; x++)
        {
            matrix[y * width + x] = 0;
        }
    }
}

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

int main() {
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

    int count = 0;
    while (running && count < 100) {
        count ++;

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