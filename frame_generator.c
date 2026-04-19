#include "frame_generator.h"

frame_buffer_t frames;
ws2811_led_t active_color[WIDTH_BLOCK];
ws2811_led_t inactive_color[WIDTH_BLOCK];
// Build the colored row — one solid color across the full width
ws2811_led_t color_row[4][WIDTH];  
// Build the colored row — one solid color across the full width
ws2811_led_t color_row[4][WIDTH];  
ws2811_led_t status[WIDTH_BLOCK];
size_t frame_count;


void frame_buffer_init(frame_buffer_t *buf)
{
    buf->frames = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

void frame_buffer_free(frame_buffer_t *buf)
{
    free(buf->frames);
    buf->frames = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

int frame_buffer_push(frame_buffer_t *buf, frame_t frame)
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

int parse_csv_frames(const char *filename, frame_buffer_t *out)
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


void init_frame()
{
    frame_buffer_init(&frames);

    if (parse_csv_frames("LetitBe.csv", &frames) != 0) {
        fprintf(stderr, "Failed to parse CSV\n");
        return 1;
    }
    
    for (int y = 0 ; y < WIDTH_BLOCK ; y++)
    {
       active_color[y] = dotcolors[5];
    	
    }
    
    for (int y = 0 ; y < WIDTH_BLOCK ; y++)
    {
       inactive_color[y] = dotcolors[6];
    	
    }
    
    for (int y = 0 ; y < 4 ; y++)
    {
        for (int x = 0; x < WIDTH_BLOCK; x++) {
        	color_row[y][x] = dotcolors[y];
    	}
    }
}

void render_frame(bool active_row , uint8_t active_lane)
{ 
  if(frame_count < frames.count)
  {
    for (int lane = 0; lane < 4; lane++) {
	if (frames.frames[i].lane[lane]) {
	    grid_insert_lane(color_row[lane], lane);
	    //grid_set_bottom_lane(active_color,lane);

	} else {
	    grid_insert_lane(blank_row, lane);
            //grid_set_bottom_lane(inactive_color,lane);
	}
    }
    grid_get_bottom_lane(status,0);
    printf("%d %d %d %d\r\n" , status[0],status[1],status[2],status[3]);

    if (render_led_grid() != 0) {
	break;
    }
    frame_count++;
  }

}

