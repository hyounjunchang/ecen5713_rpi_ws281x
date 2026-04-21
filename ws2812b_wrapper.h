#include "ws2811.h"
#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"

#ifndef WS2812B_WRAPPER_H
#define WS2812B_WRAPPER_H

#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                10 // GPIO 10 for SPI MOSI, pin 19 for Pi 4B
#define DMA                     10

//(16x16 matrix is RGB)
#define STRIP_TYPE            WS2811_STRIP_RGB		// WS2812/SK6812RGB integrated chip+leds 
//#define STRIP_TYPE              WS2811_STRIP_GBR		// WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE            SK6812_STRIP_RGBW		// SK6812RGBW (NOT SK6812RGB)

#define WIDTH_BLOCK                   4
#define WIDTH                   16
#define HEIGHT                  16
#define LED_COUNT               (WIDTH * HEIGHT)


typedef enum {
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    LIGHTBLUE,
    BLUE,
    PURPLE,
    PINK,
    COLOR_COUNT
} Default_Color;

extern ws2811_led_t dotcolors[];
extern ws2811_t ledstring;

// LED grid functions: see main.c of this repo for usage examples
int init_led_grid(); // Initializes LED grid for use (using SPI bus on GPIO 10) with all LEDs turned off
void free_led_grid(); // Frees memory and bus for later reuse
int render_led_grid(); // Renders matrix buffer
int clear_led_grid(); // turns off all LEDs on grid
void grid_insert_top_row(ws2811_led_t* colors); // Inserts colors on the TOP row specified by input
void grid_insert_lane(ws2811_led_t* colors , uint8_t lane);
void grid_set_bottom_lane(ws2811_led_t* colors, uint8_t lane);
void grid_get_bottom_lane(ws2811_led_t* status, uint8_t lane);
// Helper functions for setting the grid
// Uses buffer "matrix" ==> color matrix to be rendered on grid
void matrix_render(); // Renders matrix color buffer to LED grid
void matrix_reorder_to_pcb(); // Reorders matrix to PCB order for rendering
void matrix_clear(); // sets all matrix to 0;
#endif
