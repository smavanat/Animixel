#include "input.h"
#include "GLFW/glfw3.h"

void input_handler_init(Input_Handler *ih) {
    ih->mouse_x = 0;
    ih->mouse_y = 0;

    for(int i = 0; i < NUM_KEYS; i++) {
        ih->key_status[i] = KEY_RELEASED;
    }
}

void glfw_mouse_callback(GLFWwindow *window, int key, int action, int mods) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED) && cursor_in_bounds(&handler, 0, 0, width, height) && key < 3) {
        int idx = MOUSE_BUTTON_LEFT+key;
        if(action == GLFW_PRESS) {
            if(handler.key_status[idx] == KEY_JUST_PRESSED) {
                handler.key_status[idx] = KEY_PRESSED;
            }
            else if(handler.key_status[idx] == KEY_RELEASED || handler.key_status[idx] == KEY_JUST_RELEASED) {
                handler.key_status[idx] = KEY_JUST_PRESSED;
            }
        }
        else {
            if(handler.key_status[idx] == KEY_JUST_PRESSED || handler.key_status[idx] == KEY_PRESSED) {
                handler.key_status[idx] = KEY_JUST_RELEASED;
            }
            else {
                handler.key_status[idx] = KEY_RELEASED;
            }
        }
    }
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        int idx = -1;
        if(key >= 65  && key <= 90) {
            idx = key-GLFW_KEY_A + KEY_A;
        }
        else if (key >= 48 && key <= 57) {
            idx = key-GLFW_KEY_0+KEY_0;
        }
        else if(key == GLFW_KEY_MINUS) {
            idx = KEY_MINUS;
        }
        else if(key == GLFW_KEY_EQUAL) {
            idx = KEY_PLUS;
        }

        if(idx == -1) return;

        if(action == GLFW_PRESS) {
            if(handler.key_status[idx] == KEY_JUST_PRESSED) {
                handler.key_status[idx] = KEY_PRESSED;
            }
            else if(handler.key_status[idx] == KEY_RELEASED || handler.key_status[idx] == KEY_JUST_RELEASED) {
                handler.key_status[idx] = KEY_JUST_PRESSED;
            }
        }
        else if (action == GLFW_REPEAT) {
            if(handler.key_status[idx] != KEY_PRESSED) {
                handler.key_status[idx] = KEY_PRESSED;
            }
        }
        else {
            if(handler.key_status[idx] == KEY_JUST_PRESSED || handler.key_status[idx] == KEY_PRESSED) {
                handler.key_status[idx] = KEY_JUST_RELEASED;
            }
            else {
                handler.key_status[idx] = KEY_RELEASED;
            }
        }
    }
}

void update_key_state(Input_Handler *ih) {
    for(int i = 0; i < NUM_KEYS; i++) {
        if(ih->key_status[i] == KEY_JUST_PRESSED) ih->key_status[i] = KEY_PRESSED;
        if(ih->key_status[i] == KEY_JUST_RELEASED) ih->key_status[i] = KEY_RELEASED;
    }
}

void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        handler.prev_x = handler.mouse_x;
        handler.prev_y = handler.mouse_y;
        handler.mouse_x = xpos;
        handler.mouse_y = ypos;
    }
}

int cursor_in_bounds(Input_Handler *ih, float min_x, float min_y, float max_x, float max_y) {
    if(ih->mouse_x < min_x || ih->mouse_x >= max_x) return 0;
    if(ih->mouse_y < min_y || ih->mouse_y >= max_y) return 0;
    return 1;
}

