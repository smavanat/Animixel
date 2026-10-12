#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>
#include <stddef.h>
#include <bob.h>

typedef struct {
    uint32_t *pixels;
    BOB_Pixelbuffer_Handle pb;
    size_t width;
    size_t height;
    size_t scale;
} Canvas;

void canvas_init(Canvas *c, size_t width, size_t height, size_t scale, uint32_t default_colour, BOB_Renderer_Handle r);
void canvas_free(Canvas *c, BOB_Renderer_Handle r);
void canvas_draw(Canvas *c, BOB_Renderer_Handle renderer);
void canvas_set_pixel(Canvas *c, uint32_t x, uint32_t y, uint32_t colour);
void canvas_clear_pixel(Canvas *c, uint32_t x, uint32_t y);
void canvas_print(Canvas *c);
void canvas_bresenham(Canvas *c, uint32_t sx, uint32_t sy, uint32_t ex, uint32_t ey, uint32_t colour);

#endif
