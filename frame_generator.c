#include "frame_generator.h"
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


