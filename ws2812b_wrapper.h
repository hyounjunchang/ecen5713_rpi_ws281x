#include "ws2811.h"

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
} Color;

extern ws2811_led_t dotcolors[];
extern ws2811_t ledstring;

int init_led_grid();
void free_led_grid();
int render_led_grid();
int clear_led_grid();
void matrix_insert_top_row(ws2811_led_t* colors);
void matrix_render();
void matrix_reorder_to_pcb();
void matrix_clear();


#endif