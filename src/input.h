/**
 * @file input.h
 * @brief Contains structs/functions for handling user input
 */

#ifndef __INPUT_H__
#define __INPUT_H__

/// Enum to hold the state of the keys
typedef enum {
    KEY_JUST_PRESSED, ///< Was pressed this frame
    KEY_PRESSED, ///< Held on from the previous frame
    KEY_RELEASED, ///< Not pressed
    KEY_JUST_RELEASED ///< Released the previous frame
} keystate;

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
} keys;

/// Struct that acts as input handling interface
typedef struct {
    keystate key_status[NUM_KEYS];  ///< Map that holds the state of all the current keys we care about
    double mouse_x;  ///< Mouse x position in the current frame
    double mouse_y;  ///< Mouse y position in the current frame
    double prev_x;   ///< Mouse x position in the previous frame
    double prev_y;   ///< Mouse y position in the previous frame
    double scroll_x; ///< Mouse horizontal scroll in the current frame
    double scroll_y; ///< Mouse vertical scroll in the current frame
} Input_Handler;

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

#endif
