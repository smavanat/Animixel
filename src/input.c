#include "input.h"

void input_handler_init(Input_Handler *ih) {
    ih->mouse_x = 0;
    ih->mouse_y = 0;

    for(int i = 0; i < NUM_KEYS; i++) {
        ih->key_status[i] = KEY_RELEASED;
    }
}

void update_key_state(Input_Handler *ih) {
    for(int i = 0; i < NUM_KEYS; i++) {
        if(ih->key_status[i] == KEY_JUST_PRESSED) ih->key_status[i] = KEY_PRESSED;
        if(ih->key_status[i] == KEY_JUST_RELEASED) ih->key_status[i] = KEY_RELEASED;
    }
}

int cursor_in_bounds(Input_Handler *ih, float min_x, float min_y, float max_x, float max_y) {
    if(ih->mouse_x < min_x || ih->mouse_x >= max_x) return 0;
    if(ih->mouse_y < min_y || ih->mouse_y >= max_y) return 0;
    return 1;
}

