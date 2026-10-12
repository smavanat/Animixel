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
    memset(c->pixels, default_colour, sizeof(uint32_t) * c->width * c->height); // Set all pixels in the struct to be the default colour
    BOB_pixelbuffer_init(r, c->width, c->height, BOB_RGBA, &c->pb);
}

void canvas_free(Canvas *c, BOB_Renderer_Handle r) {
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
    BOB_draw_pixelbuffer(c->pb, (BOB_Quad){0, 0, c->width * c->scale, c->height * c->scale}, (BOB_Quad){0, 0, c->width, c->height}, (BOB_Vector4){1,1,1,1}, 20, 0.0f);
}

void canvas_set_pixel(Canvas *c, uint32_t x, uint32_t y, uint32_t colour) {
    if(x >= c->width || y >= c->height) return; // Exit if the pixel is out of the grid

    c->pixels[(c->width * y) + x] = colour;
}

void canvas_clear_pixel(Canvas *c, uint32_t x, uint32_t y) {
    canvas_set_pixel(c, x, y, 0); // Set it to be colourless and transparent so it shows the background behind it
}

void canvas_bresenham(Canvas *c, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t colour) {
    if(x1 >= c->width || y1 >= c->height || x2 >= c->width || y2 >= c->height) return;

    int m_new = 2 * (y2 - y1);
    int slope_error_new = m_new - (x2 - x1);
    for (int x = x1, y = y1; x <= x2; x++) {
        canvas_set_pixel(c, x, y, colour);

        // Add slope to increment angle formed
        slope_error_new += m_new;

        // Slope error reached limit, time to
        // increment y and update slope error.
        if (slope_error_new >= 0) {
            y++;
            slope_error_new -= 2 * (x2 - x1);
        }
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
