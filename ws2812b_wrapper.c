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

#include "ws2812b_wrapper.h"

static int width = WIDTH;
static int height = HEIGHT;
static int width_block = WIDTH_BLOCK;
ws2811_led_t dotcolors[COLOR_COUNT] =
{
    0x00200000,  // red
    0x00201000,  // yellow 
    0x00202000,  // green
    0x00002000,  // red
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

void configure_led_grid(uint8_t gpio, uint8_t w, uint8_t h, uint8_t num_players, uint8_t brightness)
{
    width = w;
    height = h;
    width_block = h/num_players/2;

    ledstring.channel[0].gpionum = gpio_pin;
    ledstring.channel[0].count = w*h;
    ledstring.channel[0].brightness = brightness;
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

void grid_insert_top_row(ws2811_led_t* colors){
    int x, y;
    // shift led down by one row
    for (y = 0; y < height - 1; y++){
        for (x = 0; x < width_block; x++){
            matrix[y * width + x] = matrix[(y + 1)*width + x];
        }
    }
    for (x = 0; x < width_block; x++){
        matrix[(height-1)*width + x] = colors[x];
    }

}

void grid_insert_lane(ws2811_led_t color, uint8_t lane)
{
  if(lane >= 0 && lane < 4)
  { 
    int x, y;
    int lane_start = lane * width_block;
    // shift led down by one row
    for (y = 0; y < height - 1; y++){
        for (x = 0; x < width_block; x++){
            matrix[y * width + lane_start + x] = matrix[(y + 1) * width + lane_start + x];
        }
    }
    for (x = 0; x < width_block; x++){
        matrix[(height - 1) * width + lane_start + x] = color;
    }
  }
  
  return;
}

void grid_set_bottom_lane(ws2811_led_t* colors, uint8_t lane)
{
    if (lane >= 4) return;

    int lane_start = lane * width_block;

    for (int x = 0; x < width_block; x++) {
        matrix[width + lane_start + x] = colors[x];
        matrix[lane_start + x] = colors[x];
    }
}

void grid_get_bottom_lane(ws2811_led_t* status, uint8_t lane)
{
    if (lane >= 4) return;
    
    if(!status) return;

    int lane_start = lane * width_block;

    for (int x = 0; x < width_block; x++) {
        status[x] = matrix[lane_start + x];
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
