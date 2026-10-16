/**
 * @file ui_core.h
 * @brief Contains structs/functions for UI using Clay
 */
#ifndef UI_CORE_H
#define UI_CORE_H

#include <bob.h>
#include <clay.h>
#include <stdint.h>
#include "canvas.h"
#include "input.h"
#include "file_io.h"

/// Global currently used to store the bitmap font used for text rendering
extern BOB_Font_Handle bitmap; //TODO: Replace this with something else so we can use different font sizes for different widgets

/// Enum used to tag different Clay Custom element types
typedef enum {
    CUSTOM_ELEMENT_TYPE_CANVAS,
} Custom_Element_Type;

extern Button_State_Manager bs_manager; //> Manages UI bs

/// Struct used to hold the data of a Clay custom element
/// as a tagged union. All of the union elements are pointers
/// to maintain a consistent struct size
typedef struct {
    Custom_Element_Type type; ///< Tag that indicates the type of the custom element data
    union {
        Canvas *canvas;     ///< Tagged union pointer element for a canvas
    };
} Custom_Element_Data;

/// Enum to hold the different states a button could be in
typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
} Button_State;

/**
 * Initialises a Button_State_Manager by setting each button to have state BUTTON_UP
 * @param bs a pointer to the Button_State_Manager to initialise
 */
// void button_state_manager_init(Button_State_Manager *bs);
// void update_sbp(Button_Type t);

/**
 * Creates a clay button widget whose size does not change
 * @param id the unique numeric id of the button to differentiate it from other buttons
 * @param width the width of the button
 * @param height the height of the button
 * @param padding the padding of the button
 * @param hover_colour the colour the button should turn when the mouse hovers over it
 * @param basic_colour the colour the button should stay when not hovered over
 * @param text the text the button should contain
 * @param button_handler callback function that should be called when the button is clicked
 * @param userData the data that should be passed to the button_handler callback
 */
void create_static_button(uint32_t id, uint32_t width, uint32_t height, uint8_t padding, Clay_Color hover_colour,
                          Clay_Color basic_colour, Clay_String text, void (*button_handler)(Clay_ElementId id,
                          Clay_PointerData info, void *userData), void *userData);

/**
 * Initialises Clay
 * @param width the width of the region the Clay UI should occupy
 * @param height the height of the region the Clay UI should occupy
 */
void clay_init(size_t width, size_t height);
/**
 * Updates the screen dimensions used by clay as well as mouse/scroll positions
 * @param ih a pointer to the input handler used to hold the new mouse/scroll positions
 * @param dt the time elapsed since last frame (in milliseconds)
 * @param width the new region width
 * @param height the new region height
 */
void clay_update_dimensions(Input_Handler *ih, float dt, size_t width, size_t height);
/**
 * Draws the Clay UI to the screen
 * @param r the handle of the BOB_Renderer used for drawing
 * @param renderCommands an array containing all of the necessary draw commands
 */
void clay_render(BOB_Renderer_Handle r, Clay_RenderCommandArray renderCommands);
/**
 * Destroys any resources used by clay during rendering
 */
void clay_free();
/**
 * Defines the Clay layout for the main editor page
 * @param dt the time elapsed since last frame (in milliseconds)
 * @param canvas a pointer to the Custom_Element_Data holding the render data for the canvas
 * @return a Clay_RenderCommandArray that holds all the draw commands for this layout
 */
Clay_RenderCommandArray editor_layout(float dt, Custom_Element_Data *canvas, File_Explorer *fe);

#endif // UI_CORE_H
