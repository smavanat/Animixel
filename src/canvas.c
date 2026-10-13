#include "canvas.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void canvas_init(Canvas *c, size_t width, size_t height, size_t scale, uint32_t default_colour, BOB_Renderer_Handle r) {
    c->width = width;
    c->height = height;
    c->scale = scale;
    c->pixels = malloc(sizeof(uint32_t) * c->width * c->height);
    c->origin_x = 0;
    c->origin_y = 0;
    memset(c->pixels, default_colour, sizeof(uint32_t) * c->width * c->height); // Set all pixels in the struct to be the default colour
    BOB_pixelbuffer_init(r, c->width, c->height, BOB_RGBA, &c->pb);
}

void canvas_free(Canvas *c) {
    free(c->pixels);
    c->pixels = NULL; // Set to null to avoid use after free issues
    BOB_pixelbuffer_free(&c->pb);
}

void canvas_draw(Canvas *c, BOB_Renderer_Handle renderer) {
    // Map pixelbuffer memory to CPU memory
    void *mapped_mem_ptr;
    size_t mem_sz;
    BOB_bind_pixelbuffer_memory(c->pb, &mapped_mem_ptr, &mem_sz);
    // Copy the data over
    memcpy(mapped_mem_ptr, c->pixels, mem_sz);
    // Send it back to GPU
    BOB_unbind_pixelbuffer_memory(c->pb);
    // Override pixelbuffer texture data with new pixels
    BOB_pixelbuffer_upload(c->pb);
    // Draw the pixelbuffer
    BOB_draw_pixelbuffer(c->pb, (BOB_Quad){c->origin_x * c->scale, c->origin_y * c->scale, c->width * c->scale, c->height * c->scale},
                         (BOB_Quad){0, 0, c->width, c->height}, (BOB_Vector4){1,1,1,1}, 20, 0.0f);
}

void canvas_set_pixel(Canvas *c, int32_t x, int32_t y, uint32_t colour) {
    if(x >= c->width || y >= c->height || x < 0 || y < 0) return; // Exit if the pixel is out of the grid

    c->pixels[(c->width * y) + x] = colour;
}

void canvas_clear_pixel(Canvas *c, int32_t x, int32_t y) {
    canvas_set_pixel(c, x, y, 0); // Set it to be colourless and transparent so it shows the background behind it
}

void canvas_bresenham(Canvas *c, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t colour) {
    int32_t dx = abs(x2 - x1);      // Difference in x values
    int32_t sx = x1 < x2 ? 1 : -1;  // Sign change (which direction is the x value of the line moving in)
    int32_t dy = -abs(y2 - y1);     // Difference in y values. Set as negative so the loop is symmetric
    int32_t sy = y1 < y2 ? 1 : -1;  // Sign change (which direction is the y value of the line moving in)
    int32_t err = dx + dy;          // Tracks how far the pixel has strayed from the ideal line
    for (;;) {
        canvas_set_pixel(c, x1, y1, colour);    // Draw the current pixel
        if (x1 == x2 && y1 == y2) break;        // Stop if we have reached the endpoint
        // Multiply error by 2. This is because in the actual algorithm the comparisons between err and dx/dy have dx/dy,
        // so this keeps everything in integer space
        int32_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }  // Line has moved far enough horizontally that a x step is due
        if (e2 <= dx) { err += dx; y1 += sy; }  // Line has moved far enough vertically that a y step is due
    }
}

void canvas_print(Canvas *c) {
    for(size_t y = 0; y < c->height; y++) {
        for(size_t x = 0; x < c->width; x++) {
            printf("%u ", c->pixels[(c->width * y) + x]);
        }
        printf("\n");
    }
}

void canvas_dump_set(Canvas *c) {
    size_t count = 0;
    for (size_t y = 0; y < c->height; y++)
        for (size_t x = 0; x < c->width; x++)
            if (c->pixels[y * c->width + x] == 0) {
                printf("(%zu, %zu)\n", x, y);
                count++;
            }
    printf("%zu pixels set\n", count);
}
