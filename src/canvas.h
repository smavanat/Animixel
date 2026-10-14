/**
 * @file canvas.h
 * @brief Contains the definition for a Canvas and its associated functions
 */

#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>
#include <stddef.h>
#include <bob.h>

/// Struct that describes a drawing canvas
typedef struct {
    uint32_t *pixels;           ///< Array holding the pixel data
    uint32_t *prev_pixels;      ///< Pixel data from before shape drawing started
    BOB_Pixelbuffer_Handle pb;  ///< Reference handle to the pixelbuffer that actually renders the canvas data to the screen
    size_t width;               ///< Width of the canvas
    size_t height;              ///< Height of the canvas
    size_t scale;               ///< Scale factor of the canvas. Used when rendering
    uint32_t origin_x;          ///< Top left x coord
    uint32_t origin_y;          ///< Top left y coord
    uint32_t shape_x;           ///< X coord from where a shape drawing started
    uint32_t shape_y;           ///< Y coord from where a shape drawing started
} Canvas;

/**
 * Initialises a Canvas. Does not allocate memory for the struct itself (this must be done beforehand
 * using either a local struct variable or a heap allocation), but will allocate memory for the pixels
 * buffer and will create the PixelBuffer via a call to BOB
 * @param c a pointer to a Canvas to be initialised
 * @param width the desired width of the Canvas
 * @param height the desired height of the Canvas
 * @param scale the desired scale factor of the Canvas
 * @param default_colour the background colour the Canvas should default to when first drawn
 * @param r the handle of the BOB_Renderer the Canvas should use to generate its pixelbuffer
 */
void canvas_init(Canvas *c, size_t width, size_t height, size_t scale, uint32_t default_colour, BOB_Renderer_Handle r);
/**
 * Frees all memory used by member variables of a Canvas outside of the struct itself, but does not
 * destroy the memory region occupied by the struct
 * @param c a pointer to the Canvas whole members must be freed
 */
void canvas_free(Canvas *c);
/**
 * Draws a canvas to the screen. Updates the Canvas' pixelbuffer with the new pixel values from
 * the pixels array. Determines the size of the destination quad using the width, height and scale member variables
 * @param c a pointer to the Canvas to be drawn
 * @param r a handle to the renderer that was used to create the Canvas' pixelbuffer
 */
void canvas_draw(Canvas *c, BOB_Renderer_Handle renderer);
/**
 * Sets a pixel in the Canvas. Only updates the pixels array, not the pixelbuffer
 * @param c a pointer to the Canvas where the pixel should be set
 * @param x the x coordinate of the pixel
 * @param y the y coordinate of the pixel
 * @param colour the new colour the pixel should be set to
 */
void canvas_set_pixel(Canvas *c, int32_t x, int32_t y, uint32_t colour);
/**
 * Uses Bresenham's line algorithm to draw a line between two points on a Canvas.
 * @param c a pointer to the Canvas where the line should be drawn
 * @param sx the starting x coord of the line
 * @param sy the starting y coord of the line
 * @param ex the ending x coord of the line
 * @param ey the ending y coord of the line
 * @param colour the colour the line whould be drawn in
 */
void canvas_bresenham(Canvas *c, int32_t sx, int32_t sy, int32_t ex, int32_t ey, uint32_t colour);
void canvas_begin_shape(Canvas *c, int32_t sx, int32_t sy);
void canvas_end_shape(Canvas *c);
void canvas_draw_line(Canvas *c, int32_t sx, int32_t sy, int32_t px, int32_t py);
void canvas_draw_filled_rect(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);
void canvas_draw_empty_rect(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);
void canvas_draw_filled_circle(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);
void canvas_draw_empty_circle(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);

/**
 * Prints the pixels array of the Canvas to the terminal. Used for debugging and
 * should not be used in production code
 * @param c a pointer to the Canvas whose pixels should be printed
 */
void canvas_print(Canvas *c);
/**
 * Prints the set pixels array of the Canvas to the terminal. Used for debugging and
 * should not be used in production code. Preffered over canvas_print
 * for large sizes where printing the full Canvas would be unreadable
 * @param c a pointer to the Canvas whose set pixels should be printed
 */
void canvas_dump_set(Canvas *c);

#endif
