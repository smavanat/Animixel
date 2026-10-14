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
 * buffer and will create the PixelBuffer via a call to BOB.
 * Sets origin_x/y to default values of 0 and shape_x/y to sentinel values of UINT32_MAX
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
/**
 * Function to be called before shape drawing starts.
 * Sets the start position of the shape (where the user first clicked) in the Canvas
 * @param c a pointer to the Canvas where the shape drawing started
 * @param sx the x coordinate of the shape start position
 * @param sy the y coordinate of the shape start position
 */
void canvas_begin_shape(Canvas *c, int32_t sx, int32_t sy);
/**
 * Ends shape drawing by resetting the Canvas' shape_x/y
 * member variables to the sentinel value of UINT32_MAX
 * @param c a pointer to Canvas where the shape drawing finished
 */
void canvas_end_shape(Canvas *c);
/**
 * Draws a line by running two passes of Bresenham's algorithm.
 * The first clears the line drawn between (start_x, start_y) and
 * (px, py) to the colours stored in the prev_pixels snapshot
 * The second draws a new line between (start_x, start_y) and (sx, sy)
 * @param c a pointer to the canvas where the line should be drawn
 * @param sx the x coordinate of the line endpoint this frame
 * @param sy the y coordinate of the line endpoint this frame
 * @param px the x coordinate of the line endpoint last frame
 * @param py the y coordinate of the line endpoint last frame
 */
void canvas_draw_line(Canvas *c, int32_t sx, int32_t sy, int32_t px, int32_t py);
/**
 * Draws a filled rect in two passes of the Canvas grid.
 * The first clears the rect drawn between (start_x, start_y) and
 * (px, py) to the colours stored in the prev_pixels snapshot.
 * The second draws a new rect between (start_x, start_y) and (sx, sy)
 * @param c a pointer to the canvas where the rect should be drawn
 * @param sx the x coordinate of the rect endpoint this frame
 * @param sy the y coordinate of the rect endpoint this frame
 * @param px the x coordinate of the rect endpoint last frame
 * @param py the y coordinate of the rect endpoint last frame
 */
void canvas_draw_filled_rect(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);
/**
 * Draws a empty rect in two passes of the Canvas grid.
 * The first clears the rect drawn between (start_x, start_y) and
 * (px, py) to the colours stored in the prev_pixels snapshot.
 * The second draws a new rect between (start_x, start_y) and (sx, sy)
 * @param c a pointer to the canvas where the rect should be drawn
 * @param sx the x coordinate of the rect endpoint this frame
 * @param sy the y coordinate of the rect endpoint this frame
 * @param px the x coordinate of the rect endpoint last frame
 * @param py the y coordinate of the rect endpoint last frame
 */
void canvas_draw_empty_rect(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);
/**
 * Draws a filled circle in two passes of the Canvas grid.
 * The first clears the circle with radius = sqrt(pow(px - start_x, 2), pow(py - start_y, 2))
 * to the colours stored in the prev_pixels snapshot
 * The second draws a new circle with radius = sqrt(pow(sx - start_x, 2), pow(sy - start_y, 2))
 * @param c a pointer to the canvas where the circle should be drawn
 * @param sx the x coordinate of the mouse pointer this frame
 * @param sy the y coordinate of the mouse pointer this frame
 * @param px the x coordinate of the mouse pointer last frame
 * @param py the y coordinate of the mouse pointer last frame
 */
void canvas_draw_filled_circle(Canvas *c, int32_t ex, int32_t ey, int32_t px, int32_t py);
/**
 * Draws an empty circle in two passes of the Canvas grid using the Midpoint Circle Algorithm.
 * The first clears the circle with radius = sqrt(pow(px - start_x, 2), pow(py - start_y, 2))
 * to the colours stored in the prev_pixels snapshot
 * The second draws a new circle with radius = sqrt(pow(sx - start_x, 2), pow(sy - start_y, 2))
 * @param c a pointer to the canvas where the circle should be drawn
 * @param sx the x coordinate of the mouse pointer this frame
 * @param sy the y coordinate of the mouse pointer this frame
 * @param px the x coordinate of the mouse pointer last frame
 * @param py the y coordinate of the mouse pointer last frame
 */
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
