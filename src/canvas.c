#include "canvas.h"
#include "math_utils.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void canvas_init(Canvas *c, size_t width, size_t height, size_t scale, uint32_t default_colour, BOB_Renderer_Handle r) {
    c->width = width;
    c->height = height;
    c->scale = scale;
    c->pixels = malloc(sizeof(uint32_t) * c->width * c->height);
    c->prev_pixels = malloc(sizeof(uint32_t) * c->width * c->height);
    c->origin_x = 0;
    c->origin_y = 0;
    c->shape_x = UINT32_MAX;
    c->shape_y = UINT32_MAX;
    memset(c->pixels, default_colour, sizeof(uint32_t) * c->width * c->height); // Set all pixels in the struct to be the default colour
    BOB_pixelbuffer_init(r, c->width, c->height, BOB_RGBA, &c->pb);
}

void canvas_free(Canvas *c) {
    free(c->pixels);
    c->pixels = NULL; // Set to null to avoid use after free issues
    free(c->prev_pixels);
    c->prev_pixels = NULL; // Set to null to avoid use after free issues
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

void canvas_bresenham(Canvas *c, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t colour) {
    // Clamp the values in range to avoid extraneous calculations
    x1 = clampi(x1, 0, c->width - 1);
    x2 = clampi(x2, 0, c->width - 1);
    y1 = clampi(y1, 0, c->height - 1);
    y2 = clampi(y2, 0, c->height - 1);

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

void canvas_begin_shape(Canvas *c, int32_t sx, int32_t sy) {
    c->shape_x = (uint32_t)clampi(sx, 0, c->width - 1);
    c->shape_y = (uint32_t)clampi(sy, 0, c->width - 1);

    memcpy(c->prev_pixels, c->pixels, sizeof(uint32_t) * c->width * c->height);
}

void canvas_end_shape(Canvas *c) {
    c->shape_x = UINT32_MAX;
    c->shape_y = UINT32_MAX;
}

void canvas_draw_line(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py) {
    if(c->shape_x >= c->width || c->shape_y >= c->height) return; // Early exit if the shape start is invalid

    // Find the old line pixels manually using bresenham's
    // This is faster than just memcpying the memory over
    // since it doesn't make any syscalls and doesn't move any memory around
    int32_t ox = c->shape_x;
    int32_t oy = c->shape_y;
    ex = clampi(ex, 0, c->width - 1);
    px = clampi(px, 0, c->width - 1);
    ey = clampi(ey, 0, c->height - 1);
    py = clampi(py, 0, c->height - 1);

    int32_t dx = abs(px - ox);      // Difference in x values
    int32_t sx = ox < px ? 1 : -1;  // Sign change (which direction is the x value of the line moving in)
    int32_t dy = -abs(py - oy);     // Difference in y values. Set as negative so the loop is symmetric
    int32_t sy = oy < py ? 1 : -1;  // Sign change (which direction is the y value of the line moving in)
    int32_t err = dx + dy;          // Tracks how far the pixel has strayed from the ideal line
    for (;;) {
        canvas_set_pixel(c, ox, oy, c->prev_pixels[(oy * c->width) + ox]);    // Draw the current pixel
        if (ox == px && oy == py) break;        // Stop if we have reached the endpoint
        // Multiply error by 2. This is because in the actual algorithm the comparisons between err and dx/dy have dx/dy,
        // so this keeps everything in integer space
        int32_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; ox += sx; }  // Line has moved far enough horizontally that a x step is due
        if (e2 <= dx) { err += dx; oy += sy; }  // Line has moved far enough vertically that a y step is due
    }

    // Then update the new line using normal bresenham's
    canvas_bresenham(c, c->shape_x, c->shape_y, ex, ey, UINT32_MAX);
}

void canvas_draw_filled_rect(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py) {
    if(c->shape_x >= c->width || c->shape_y >= c->height) return; // Early exit if the shape start is invalid

    ex = clampi(ex, 0, c->width - 1);
    px = clampi(px, 0, c->width - 1);
    ey = clampi(ey, 0, c->height - 1);
    py = clampi(py, 0, c->height - 1);

    int32_t sx = (int32_t)c->shape_x;
    int32_t sy = (int32_t)c->shape_y;

    // Replace the pixels in the old rectangle
    for(int32_t y = mini(sy, py); y <= maxi(sy, py); y++) {
        for(int32_t x = mini(sx, px); x <= maxi(sx, px); x++) {
            canvas_set_pixel(c, x, y, c->prev_pixels[(y * c->width) + x]);
        }
    }

    // Fill the new rectangle
    for(int32_t y = mini(sy, ey); y <= maxi(sy, ey); y++) {
        for(int32_t x = mini(sx, ex); x <= maxi(sx, ex); x++) {
            canvas_set_pixel(c, x, y, UINT32_MAX);
        }
    }
}

void canvas_draw_empty_rect(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py) {
    if(c->shape_x >= c->width || c->shape_y >= c->height) return; // Early exit if the shape start is invalid

    ex = clampi(ex, 0, c->width - 1);
    px = clampi(px, 0, c->width - 1);
    ey = clampi(ey, 0, c->height - 1);
    py = clampi(py, 0, c->height - 1);

    int32_t sx = (int32_t)c->shape_x;
    int32_t sy = (int32_t)c->shape_y;

    int32_t start_x = mini(sx, px);
    int32_t end_x = maxi(sx, px);
    int32_t start_y = mini(sy, py);
    int32_t end_y = maxi(sy, py);

    // Replace the pixels in the old rectangle
    for(int32_t x = start_x; x <= end_x; x++) {
        canvas_set_pixel(c, x, start_y, c->prev_pixels[(start_y * c->width) + x]);
        canvas_set_pixel(c, x, end_y, c->prev_pixels[(end_y * c->width) + x]);
    }
    for(int32_t y = start_y+1; y < end_y; y++) {
        canvas_set_pixel(c, start_x, y, c->prev_pixels[(y * c->width) + start_x]);
        canvas_set_pixel(c, end_x, y, c->prev_pixels[(y * c->width) + end_x]);
    }

    start_x = mini(sx, ex);
    end_x = maxi(sx, ex);
    start_y = mini(sy, ey);
    end_y = maxi(sy, ey);
    // Set the pixels in the new rectangle
    for(int32_t x = start_x; x <= end_x; x++) {
        canvas_set_pixel(c, x, start_y, UINT32_MAX);
        canvas_set_pixel(c, x, end_y, UINT32_MAX);
    }
    for(int32_t y = start_y+1; y < end_y; y++) {
        canvas_set_pixel(c, start_x, y, UINT32_MAX);
        canvas_set_pixel(c, end_x, y, UINT32_MAX);
    }
}

static void circle_plot(Canvas *c, int32_t x, int32_t y, uint32_t colour, int restore) {
    if (x < 0 || y < 0 || x >= (int32_t)c->width || y >= (int32_t)c->height) return;
    canvas_set_pixel(c, x, y, restore ? c->prev_pixels[(size_t)y * c->width + x] : colour);
}

static void circle_points(Canvas *c, int32_t cx, int32_t cy, int32_t x, int32_t y,
                          uint32_t colour, int restore) {
    circle_plot(c, cx + x, cy + y, colour, restore);
    circle_plot(c, cx - x, cy + y, colour, restore);
    circle_plot(c, cx + x, cy - y, colour, restore);
    circle_plot(c, cx - x, cy - y, colour, restore);
    if (x != y) {
        circle_plot(c, cx + y, cy + x, colour, restore);
        circle_plot(c, cx - y, cy + x, colour, restore);
        circle_plot(c, cx + y, cy - x, colour, restore);
        circle_plot(c, cx - y, cy - x, colour, restore);
    }
}

static void empty_circle_pass(Canvas *c, int32_t cx, int32_t cy, int32_t r, uint32_t colour, int restore) {
    if (r <= 0) { circle_plot(c, cx, cy, colour, restore); return; }

    int32_t x = r, y = 0, p = 1 - r;
    circle_points(c, cx, cy, x, y, colour, restore);   // axis points

    while (x > y) {
        y++;
        if (p <= 0) p += 2 * y + 1;
        else { x--; p += 2 * y - 2 * x + 1; }
        if (x < y) break;
        circle_points(c, cx, cy, x, y, colour, restore);
    }
}

static int32_t dist_i32(int32_t dx, int32_t dy) {
    return (int32_t)lround(sqrt((double)dx * dx + (double)dy * dy));
}

void canvas_draw_empty_circle(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py) {
    if (c->shape_x >= c->width || c->shape_y >= c->height) return;

    int32_t sx = (int32_t)c->shape_x;
    int32_t sy = (int32_t)c->shape_y;

    empty_circle_pass(c, sx, sy, dist_i32(px - sx, py - sy), 0, 1);           // undo last frame
    empty_circle_pass(c, sx, sy, dist_i32(ex - sx, ey - sy), UINT32_MAX, 0);  // draw this frame
}

static void filled_circle_pass(Canvas *c, int32_t cx, int32_t cy, int32_t r, uint32_t colour, int restore) {
    if (r <= 0) { circle_plot(c, cx, cy, colour, restore); return; }
    int32_t sr = r * r;

    for(int32_t y = 0; y <= r; y++) {
        for(int32_t x = 0; x <= r; x++) {
            if(y * y + x * x <= sr) {
                circle_plot(c, cx + x, cy + y, colour, restore);
                if (x) circle_plot(c, cx - x, cy + y, colour, restore);
                if (y) circle_plot(c, cx + x, cy - y, colour, restore);
                if (x && y) circle_plot(c, cx - x, cy - y, colour, restore);
            }
        }
    }
}

void canvas_draw_filled_circle(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py) {
    if (c->shape_x >= c->width || c->shape_y >= c->height) return;

    int32_t sx = (int32_t)c->shape_x;
    int32_t sy = (int32_t)c->shape_y;

    filled_circle_pass(c, sx, sy, dist_i32(px - sx, py - sy), 0, 1);           // undo last frame
    filled_circle_pass(c, sx, sy, dist_i32(ex - sx, ey - sy), UINT32_MAX, 0);  // draw this frame
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
