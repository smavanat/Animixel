#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define _XOPEN_SOURCE 500 //Needed to get deprecated functions
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L  // must come first
#endif

//TODO: FIX BOB VULKAN PBO

#define NDEBUG
#define BOB_INCLUDE_GLAD
// #define BOB_INCLUDE_VULKAN
#define CLAY_IMPLEMENTATION
#include "ui_core.h"
#define BOB_IMPLEMENTATION
#include <bob.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <sys/time.h>

#include "input.h"

#define FRAME_RATE 1000 / 60.0f

GLFWwindow *window;
BOB_Renderer_Handle r;
Input_Handler handler;
Button_State_Manager bs_manager;
Canvas c;
uint32_t screen_width = 800, screen_height = 600;
uint32_t last_fb_w = 800, last_fb_h = 600;
BOB_Font_Handle bitmap;

//Thank you Bernardo: https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
#ifdef WIN32
#include <windows.h>
#else
#include <time.h>   // for nanosleep
#include <unistd.h> // for usleep
#endif

/**
 * Cross-platform sleep function
 * @param milliseconds the time the thread should sleep for in milliseconds
 */
void sleep_ms(int milliseconds){
#ifdef WIN32
    Sleep(milliseconds);
#else
#if defined(_POSIX_VERSION) && (POSIX_VERSION >= 199309L)
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
#endif
}

//GLFW callback for when the window is resized
//Updates the width and height in the renderer so that the projection matrix
//(and the objects being rendered) can be adjusted to fit the new screen size
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    //Update viewport dimensions
    #ifdef BOB_INCLUDE_GLAD
    glViewport(0, 0, width, height);
    #endif
    glfwGetWindowSize(window, &screen_width, &screen_height);
    BOB_renderer_update_dimensions(r, screen_width, screen_height, width, height);
}

/**
 * Callback function to be used by GLFW when a mouse button's state changes.
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_JUST_PRESSED.
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_PRESSED or KEY_JUST_PRESSED,
 * sets the state to KEY_PRESSED.
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_RELEASED.
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_PRESSED or KEY_PRESSED,
 * sets the state to KEY_JUST_RELEASED.
 * @param window a pointer to the GLFWwindow where the mouse button change occured
 * @param key the mouse button whose state has changed
 * @param action the new state of the mouse button
 * @param mods modifier bits. Not used by this callback implementation
 */
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

/**
 * Callback function to be used by GLFW when the scroll state changes.
 * @param window a pointer to the GLFWwindow where the scroll state changed
 * @param xpos the new scroll horizontal position
 * @param ypos the new scroll vertical position
 */
void glfw_scroll_callback(GLFWwindow *window, double xpos, double ypos) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        handler.scroll_x = xpos;
        handler.scroll_y = ypos;
    }
}

/**
 * Callback function to be used by glfw when the cursor position changes.
 * @param window a pointer to the GLFWwindow where the mouse position changed
 * @param xpos the new x position
 * @param ypos the new y position
 */
void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        // handler.prev_x = handler.mouse_x;
        // handler.prev_y = handler.mouse_y;
        handler.mouse_x = xpos;
        handler.mouse_y = ypos;
    }
}

/**
 * Callback function to be used by glfw when a key's state changes.
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_JUST_PRESSED.
 * If the new state is GLFW_PRESS and the old state stored in the input_handler was KEY_PRESSED or KEY_JUST_PRESSED,
 * sets the state to KEY_PRESSED.
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_RELEASED or KEY_RELEASED,
 * sets the state to KEY_RELEASED.
 * If the new state is GLFW_RELEASE and the old state stored in the input_handler was KEY_JUST_PRESSED or KEY_PRESSED,
 * sets the state to KEY_JUST_RELEASED.
 * If the new state is GLFW_REPEAT set the state to KEY_PRESSED.
 * @param window a pointer to the GLFWwindow where the key change occured
 * @param key the key whose state has changed
 * @param scancode the hardware specific scancode of the key. Not used by this callback implementation
 * @param action the new state of the key
 * @param mods modifier bits. Not used by this callback implementation
 */
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


#ifdef BOB_INCLUDE_VULKAN
uint8_t create_window_surface(VkInstance instance, VkSurfaceKHR *surface) {
    if(glfwCreateWindowSurface(instance, window, NULL, surface)){
        printf("Failed to create window surface\n");
        return 0;
    }
    return 1;
}
#endif

//Function that initialises GLFW and glad
int init(GLFWwindow **window) {
    //Initialising GLFW:
    glfwInit();
    #ifdef BOB_INCLUDE_GLAD
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24); //Necessary for enabling the depth buffer

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    *window = glfwCreateWindow(800, 600, "BOB", NULL, NULL);

    if(*window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return 0;
    }

    //Setting callback functions
    glfwMakeContextCurrent(*window);
    glfwSwapInterval(0); // Turn off Vsync to stop screen tearing when resizing
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
    glfwSetKeyCallback(*window, glfw_key_callback);
    glfwSetMouseButtonCallback(*window, glfw_mouse_callback);
    glfwSetCursorPosCallback(*window, glfw_cursor_pos_callback);
    glfwSetScrollCallback(*window, glfw_scroll_callback);

    input_handler_init(&handler);

    if(!BOB_init((GLADloadproc)glfwGetProcAddress, 1)) {
        glfwTerminate();
        return 0;
    }
    if(!BOB_create_opengl_renderer(BOB_DEFAULT_ATLAS_CAPACITY, BOB_DEFAULT_PIXELBUFFER_CAPACITY,
                                   BOB_DEFAULT_TEX_CAPACITY, BOB_DEFAULT_MATERIAL_CAPACITY, BOB_DEFAULT_FONT_CAPACITY,
                                   800, 600, BOB_DEFAULT_VERTEX_CAPACITY, BOB_DEFAULT_INDEX_CAPACITY,
                                   BOB_DEFAULT_DRAW_CALL_CAPACITY, BOB_DEFAULT_STACK_CAPACITY, BOB_MAX_UNIFORMS, &r)) {
        BOB_terminate();
        glfwTerminate();
        return 0;
    }
    #endif

    #ifdef BOB_INCLUDE_VULKAN
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Set to NO_API so that it uses Vulkan instead of OpenGL
    *window = glfwCreateWindow(800, 600, "BOB", NULL, NULL);

    if(*window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return 0;
    }

    //Setting callback functions
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);

    //Get the required extensions from GLFW
    uint32_t glfw_extension_count = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if(glfwExtensions == NULL) {
        printf("Failed to get required GLFW extensions\n");
        return 0;
    }

    if(!BOB_init(glfwExtensions, glfw_extension_count, 1)) {
        glfwTerminate();
        return 0;
    }

    int width, height;
    glfwGetFramebufferSize(*window, &width, &height);
    if(!BOB_create_vulkan_renderer(BOB_DEFAULT_ATLAS_CAPACITY, BOB_DEFAULT_PIXELBUFFER_CAPACITY, BOB_DEFAULT_TEX_CAPACITY,
                                   BOB_DEFAULT_MATERIAL_CAPACITY, BOB_DEFAULT_FONT_CAPACITY, 800, 600, width, height,
                                   BOB_DEFAULT_VERTEX_CAPACITY, BOB_DEFAULT_INDEX_CAPACITY, BOB_DEFAULT_DRAW_CALL_CAPACITY,
                                   BOB_DEFAULT_STACK_CAPACITY, BOB_MAX_UNIFORMS, &create_window_surface, &r)) {
        glfwTerminate();
        return 0;
    }
    #endif

    return 1;
}

void process_input(GLFWwindow *window) {
    if(glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        if(handler.key_status[KEY_D] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_DRAW) ? BUTTON_NONE : BUTTON_DRAW;
            update_sbp(BUTTON_DRAW);
        }
        if(handler.key_status[KEY_E] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_ERASE) ? BUTTON_NONE : BUTTON_ERASE;
            update_sbp(BUTTON_ERASE);
        }
        if(handler.key_status[KEY_S] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_SHAPE) ? BUTTON_NONE : BUTTON_SHAPE;
            update_sbp(BUTTON_SHAPE);
        }
        if(handler.key_status[KEY_L] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_LINE) ? BUTTON_NONE : BUTTON_LINE;
            update_sbp(BUTTON_LINE);
        }
        if(handler.key_status[KEY_R] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_FILLED_RECT) ? BUTTON_NONE : BUTTON_FILLED_RECT;
            update_sbp(BUTTON_FILLED_RECT);
        }
        if(handler.key_status[KEY_T] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_EMPTY_RECT) ? BUTTON_NONE : BUTTON_EMPTY_RECT;
            update_sbp(BUTTON_EMPTY_RECT);
        }
        if(handler.key_status[KEY_C] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_FILLED_CIRCLE) ? BUTTON_NONE : BUTTON_FILLED_CIRCLE;
            update_sbp(BUTTON_FILLED_CIRCLE);
        }
        if(handler.key_status[KEY_V] == KEY_JUST_PRESSED) {
            bs_manager.stb = (bs_manager.stb == BUTTON_EMPTY_CIRCLE) ? BUTTON_NONE : BUTTON_EMPTY_CIRCLE;
            update_sbp(BUTTON_EMPTY_CIRCLE);
        }
        if((bs_manager.stb == BUTTON_DRAW || bs_manager.stb == BUTTON_ERASE) &&
            (handler.key_status[MOUSE_BUTTON_LEFT] == KEY_PRESSED || handler.key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_PRESSED)) {
            int32_t ex = (int32_t)(handler.mouse_x / c.scale) - (int32_t)c.origin_x;
            int32_t ey = (int32_t)(handler.mouse_y / c.scale) - (int32_t)c.origin_y;
            int32_t sx = (int32_t)(handler.prev_x / c.scale) - (int32_t)c.origin_x;
            int32_t sy = (int32_t)(handler.prev_y / c.scale) - (int32_t)c.origin_y;

            // Check to make sure the mouse is actually in the canvas before drawing anything
            if((ex >= 0 && ex < c.width && ey >= 0 && ey < c.height)) {
                uint32_t colour = (bs_manager.stb == BUTTON_DRAW) ? UINT32_MAX : 0;
                canvas_set_pixel(&c, ex, ey, colour);

                // Only use bresenham if the press was continuing from the previous frame
                if(handler.key_status[MOUSE_BUTTON_LEFT] == KEY_PRESSED) {
                    canvas_bresenham(&c, sx, sy, ex, ey, colour);
                }
            }
        }
        // Need to set prev_x here so that it is based on the mouse position last frame not last event
        // (otherwise will get incomplete lines from Bresenham)
        handler.prev_x = handler.mouse_x;
        handler.prev_y = handler.mouse_y;
    }
}

uint32_t load_tex(char *path, BOB_Font_Handle font) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        switch (nrChannels) {
            case 1: format = BOB_RED; break;
            case 2: format = BOB_RG; break;
            case 3: format = BOB_RGB; break;
            case 4: format = BOB_RGBA; break;
        }

        BOB_add_font_page(font, width, height, data, format);
    }
    else {
        fprintf(stderr, "Failed to load texture at path %s\n", path);
        return 0;
    }
    stbi_image_free(data);

    return 1;
}

// TODO: SCREEN TEARING STILL HAPPENS AT TOP. NEED TO FIX
int main(void) {
    if(init(&window)) {
        printf("Initialised\n");

        // Initialsing the font used for text rendering
        int8_t res = BOB_load_bmf_font(r, "../data/test.fnt", BOB_BMF_TEXT, &bitmap);
        if(res <= 0) BOB_print_parsing_error();
        load_tex("../data/test.png", bitmap);

        // Initialising clay
        clay_init(screen_width, screen_height);
        button_state_manager_init(&bs_manager);
        // Setting up a custom data element to use to hold the canvas once so
        // don't recreate it every frame
        canvas_init(&c, 100, 100, 4, 0, r);
        Custom_Element_Data canvas_data = {.type = CUSTOM_ELEMENT_TYPE_CANVAS, .canvas = &c};

        // Timing between frames
        struct timeval stop, start; //Store the start and end times of a frame
        float dt = 0.0f; //Holds the time passed between frames

        // Main loop
        while(!glfwWindowShouldClose(window)) {
            gettimeofday(&start, NULL); //Getting time at start of frame
            glfwPollEvents();

            // Force window to resize if dimensions do not match to avoid screen tearing
            int win_w, win_h, fb_w, fb_h;
            glfwGetWindowSize(window, &win_w, &win_h);
            glfwGetFramebufferSize(window, &fb_w, &fb_h);

            if (win_w != screen_width || win_h != screen_height || fb_w != last_fb_w || fb_h != last_fb_h) {
                glViewport(0, 0, fb_w, fb_h);
                BOB_renderer_update_dimensions(r, win_w, win_h, fb_w, fb_h);
                screen_width = win_w;  screen_height = win_h;
                last_fb_w = fb_w;      last_fb_h = fb_h;
            }

            // Process input events and update key state
            process_input(window);
            update_key_state(&handler);

            //Clear the screen
            clay_update_dimensions(&handler, dt, screen_width, screen_height);
            Clay_RenderCommandArray render_commands = editor_layout(dt, &canvas_data);

            // Render code
            BOB_renderer_begin(r, (float[4]){1.0f, 0.0f, 1.0f, 1.0f});
                clay_render(r, render_commands);
            BOB_renderer_end(r);

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Compute time left at end of frame
            gettimeofday(&stop, NULL); //Get the time at the end of the frame
            dt = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000.0f; //Get the frame length
            // If the frame took less time than the set frame rate, wait until the time is the frame rate
            if(dt < FRAME_RATE) {
                // TODO: Unsure which one of these to use in the final version. Set it to be a macro for now
                #if 0
                // Returns early on resize/input. This is better than just capping at 60 as prevents screen tearing when resizing
                glfwWaitEventsTimeout((FRAME_RATE - dt) / 1000.0);
                // Measure the full iteration, including however long we waited
                gettimeofday(&stop, NULL);
                dt = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000.0f;
                #else
                sleep_ms((int)FRAME_RATE - dt);
                dt = FRAME_RATE;
                #endif
            }
        }

        canvas_free(&c);
    }
    glfwTerminate();
    BOB_terminate();
}
