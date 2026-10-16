/**
 * @file input.h
 * @brief Contains structs/functions for handling user input
 */

#ifndef __INPUT_H__
#define __INPUT_H__
#include <clay.h>
#include <stdint.h>
#include "canvas.h"

/// Enum to hold the state of the keys
typedef enum {
    KEY_JUST_PRESSED, ///< Was pressed this frame
    KEY_PRESSED, ///< Held on from the previous frame
    KEY_RELEASED, ///< Not pressed
    KEY_JUST_RELEASED ///< Released the previous frame
} Key_State;

/// Enum that holds all of the keys we currently want to work with
typedef enum {
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_MINUS,
    KEY_PLUS,
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    NUM_KEYS
} Keys;

/// Struct that acts as input handling interface
typedef struct {
    Key_State key_status[NUM_KEYS];  ///< Map that holds the state of all the current keys we care about
    double mouse_x;  ///< Mouse x position in the current frame
    double mouse_y;  ///< Mouse y position in the current frame
    double prev_x;   ///< Mouse x position in the previous frame
    double prev_y;   ///< Mouse y position in the previous frame
    double scroll_x; ///< Mouse horizontal scroll in the current frame
    double scroll_y; ///< Mouse vertical scroll in the current frame
} Input_Handler;

/// Enum to hold the different buttons
typedef enum {
    BUTTON_DRAW,            ///< Button to draw freehand
    BUTTON_ERASE,           ///< Button to erase
    BUTTON_SHAPE,           ///< Button to select a shape to draw with
    BUTTON_FILE,            ///< Button to open the file menu
    BUTTON_LINE,            ///< Button to start drawing a line
    BUTTON_EMPTY_RECT,      ///< Button to start drawing an empty rect
    BUTTON_FILLED_RECT,     ///< Button to start drawing a filled rect
    BUTTON_EMPTY_CIRCLE,    ///< Button to start drawing an empty circle
    BUTTON_FILLED_CIRCLE,   ///< Button to start drawing a filled circle
    BUTTON_LOAD_MENU,       ///< Button to open the file explorer in 'Load' mode
    BUTTON_SAVE_MENU,       ///< Button to open the file explorer in 'Save' mode
    BUTTON_LOAD,            ///< Button to load a file
    BUTTON_SAVE,            ///< Button to save a file
    BUTTON_DELTE,           ///< Button to delete a file
    BUTTON_CLOSE,           ///< Generic button type used by all close buttons
    BUTTON_NONE,            ///< Sentinel value to represent no button selected
} Button_Type;

/// Struct to manage the state of all the buttons similar to Input_Handler
typedef struct {
    // Button_State buttons[NUM_BUTTONS];
    Button_Type stb; ///< Selected toolbar button
    Button_Type smb; ///< Selected menu button
    Clay_String sbp; ///< Shape button pressed
    Button_Type fe_mode; ///< Should the file explorer be shown
} Button_State_Manager;

/**
 * Initialises an Input_Handler.
 * Sets the initial cursor position to (0, 0) and all of the keys to KEY_RELEASED
 * Does not allocate memory for the Input_Handler, this must be done before the function was called
 * @param ih a pointer to the Input_Handler to be initialised
 */
void input_handler_init(Input_Handler *ih);
/**
 * Updates the key state after user input processing.
 * @param ih a pointer to the Input_Handler whose key map needs updating
 */
void update_key_state(Input_Handler *ih);
/**
 * Simple checker that returns if the cursor is in the bounds [min_x, max_x), [min_y, max_y)
 * @param ih a pointer to the Input_Handler whose mouse position we are checking
 * @param min_x the minimum x value the cursor can hold
 * @param min_y the minimum y value the cursor can hold
 * @param max_x the maximum x value the cursor can hold
 * @param max_y the maximum y value the cursor can hold
 * @return 1 if the cursor is in the bounds of the window, 0 otherwise
 */
int cursor_in_bounds(Input_Handler *ih, float min_x, float min_y, float max_x, float max_y);

/**
 * Initialises a Button_State_Manager. Sets button values to BUTTON_NONE and sbp to "S"
 * @param bs a pointer to the Button_State_Manager to initialise
 */
void button_state_manager_init(Button_State_Manager *bs);
/**
 * Processes user input and updates button state
 * @param ih a pointer to the Input_Handler holding the input state
 * @param bs_manager a pointer to the Button_State_Manager holding the button state
 * @param c a pointer to a canvas that could be affected by user input
 */
void process_input(Input_Handler *ih, Button_State_Manager *bs_manager, Canvas *c);
/**
 * Helper function to update the string displayed in the Shape button
 * Called when a button/button hotkey is pressed
 * @param bs_manager a pointer to the Button_State_Manager holding the button state
 * @param t the button just pressed
 */
void update_sbp(Button_State_Manager *bs_manager, Button_Type t);

#endif
