#include "input.h"
#include "canvas.h"
#include <stdint.h>

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

void process_input(Input_Handler *ih, Button_State_Manager *bs_manager, Canvas *c) {
    if(ih->key_status[KEY_D] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_DRAW) ? BUTTON_NONE : BUTTON_DRAW;
        update_sbp(bs_manager, BUTTON_DRAW);
    }
    if(ih->key_status[KEY_E] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_ERASE) ? BUTTON_NONE : BUTTON_ERASE;
        update_sbp(bs_manager, BUTTON_ERASE);
    }
    if(ih->key_status[KEY_S] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_SHAPE) ? BUTTON_NONE : BUTTON_SHAPE;
        update_sbp(bs_manager, BUTTON_SHAPE);
    }
    if(ih->key_status[KEY_L] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_LINE) ? BUTTON_NONE : BUTTON_LINE;
        update_sbp(bs_manager, BUTTON_LINE);
    }
    if(ih->key_status[KEY_R] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_FILLED_RECT) ? BUTTON_NONE : BUTTON_FILLED_RECT;
        update_sbp(bs_manager, BUTTON_FILLED_RECT);
    }
    if(ih->key_status[KEY_T] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_EMPTY_RECT) ? BUTTON_NONE : BUTTON_EMPTY_RECT;
        update_sbp(bs_manager, BUTTON_EMPTY_RECT);
    }
    if(ih->key_status[KEY_C] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_FILLED_CIRCLE) ? BUTTON_NONE : BUTTON_FILLED_CIRCLE;
        update_sbp(bs_manager, BUTTON_FILLED_CIRCLE);
    }
    if(ih->key_status[KEY_V] == KEY_JUST_PRESSED) {
        bs_manager->stb = (bs_manager->stb == BUTTON_EMPTY_CIRCLE) ? BUTTON_NONE : BUTTON_EMPTY_CIRCLE;
        update_sbp(bs_manager, BUTTON_EMPTY_CIRCLE);
    }

    if(ih->key_status[MOUSE_BUTTON_LEFT] != KEY_RELEASED) {
        switch(bs_manager->stb) {
            case BUTTON_DRAW:
            case BUTTON_ERASE: {
                if(ih->key_status[MOUSE_BUTTON_LEFT] == KEY_PRESSED || ih->key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_PRESSED) {
                    int32_t ex = (int32_t)(ih->mouse_x / c->scale) - (int32_t)c->origin_x;
                    int32_t ey = (int32_t)(ih->mouse_y / c->scale) - (int32_t)c->origin_y;

                    // Check to make sure the mouse is actually in the canvas before drawing anything
                    if((ex >= 0 && ex < c->width && ey >= 0 && ey < c->height)) {
                        uint32_t colour = (bs_manager->stb == BUTTON_DRAW) ? UINT32_MAX : 0;
                        if(ih->key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_PRESSED)
                            canvas_set_pixel(c, ex, ey, colour);
                        // Only use bresenham if the press was continuing from the previous frame
                        else if(ih->key_status[MOUSE_BUTTON_LEFT] == KEY_PRESSED) {
                            int32_t sx = (int32_t)(ih->prev_x / c->scale) - (int32_t)c->origin_x;
                            int32_t sy = (int32_t)(ih->prev_y / c->scale) - (int32_t)c->origin_y;
                            canvas_bresenham(c, sx, sy, ex, ey, colour);
                        }
                    }
                }
                break;
            }
            case BUTTON_LINE: {
                int32_t sx = (int32_t)(ih->mouse_x / c->scale) - (int32_t)c->origin_x;
                int32_t sy = (int32_t)(ih->mouse_y / c->scale) - (int32_t)c->origin_y;
                int32_t px = (int32_t)(ih->prev_x / c->scale) - (int32_t)c->origin_x;
                int32_t py = (int32_t)(ih->prev_y / c->scale) - (int32_t)c->origin_y;
                switch(ih->key_status[MOUSE_BUTTON_LEFT]) {
                    case KEY_JUST_PRESSED: {
                        if(sx >= 0 && sx < c->width && sy >= 0 && sy < c->height)
                            canvas_begin_shape(c, sx, sy);
                        break;
                    }
                    case KEY_PRESSED: {
                        canvas_draw_line(c, sx, sy, px, py);
                        break;
                    }
                    case KEY_JUST_RELEASED: {
                        canvas_draw_line(c, sx, sy, px, py);
                        canvas_end_shape(c);
                        break;
                    }
                    default: break;
                }
                break;
            }
            case BUTTON_EMPTY_RECT: {
                int32_t sx = (int32_t)(ih->mouse_x / c->scale) - (int32_t)c->origin_x;
                int32_t sy = (int32_t)(ih->mouse_y / c->scale) - (int32_t)c->origin_y;
                int32_t px = (int32_t)(ih->prev_x / c->scale) - (int32_t)c->origin_x;
                int32_t py = (int32_t)(ih->prev_y / c->scale) - (int32_t)c->origin_y;
                switch(ih->key_status[MOUSE_BUTTON_LEFT]) {
                    case KEY_JUST_PRESSED: {
                        canvas_begin_shape(c, sx, sy);
                        break;
                    }
                    case KEY_PRESSED: {
                        canvas_draw_empty_rect(c, sx, sy, px, py);
                        break;
                    }
                    case KEY_JUST_RELEASED: {
                        canvas_draw_empty_rect(c, sx, sy, px, py);
                        canvas_end_shape(c);
                        break;
                    }
                    default: break;
                }
                break;
            }
            case BUTTON_FILLED_RECT: {
                int32_t sx = (int32_t)(ih->mouse_x / c->scale) - (int32_t)c->origin_x;
                int32_t sy = (int32_t)(ih->mouse_y / c->scale) - (int32_t)c->origin_y;
                int32_t px = (int32_t)(ih->prev_x / c->scale) - (int32_t)c->origin_x;
                int32_t py = (int32_t)(ih->prev_y / c->scale) - (int32_t)c->origin_y;
                switch(ih->key_status[MOUSE_BUTTON_LEFT]) {
                    case KEY_JUST_PRESSED: {
                        canvas_begin_shape(c, sx, sy);
                        break;
                    }
                    case KEY_PRESSED: {
                        canvas_draw_empty_rect(c, sx, sy, px, py);
                        break;
                    }
                    case KEY_JUST_RELEASED: {
                        canvas_draw_filled_rect(c, sx, sy, px, py);
                        canvas_end_shape(c);
                        break;
                    }
                    default: break;
                }
                break;
            }
            case BUTTON_EMPTY_CIRCLE: {
                int32_t sx = (int32_t)(ih->mouse_x / c->scale) - (int32_t)c->origin_x;
                int32_t sy = (int32_t)(ih->mouse_y / c->scale) - (int32_t)c->origin_y;
                int32_t px = (int32_t)(ih->prev_x / c->scale) - (int32_t)c->origin_x;
                int32_t py = (int32_t)(ih->prev_y / c->scale) - (int32_t)c->origin_y;
                switch(ih->key_status[MOUSE_BUTTON_LEFT]) {
                    case KEY_JUST_PRESSED: {
                        canvas_begin_shape(c, sx, sy);
                        break;
                    }
                    case KEY_PRESSED: {
                        canvas_draw_empty_circle(c, sx, sy, px, py);
                        break;
                    }
                    case KEY_JUST_RELEASED: {
                        canvas_draw_empty_circle(c, sx, sy, px, py);
                        canvas_end_shape(c);
                        break;
                    }
                    default: break;
                }
                break;
            }
            case BUTTON_FILLED_CIRCLE: {
                int32_t sx = (int32_t)(ih->mouse_x / c->scale) - (int32_t)c->origin_x;
                int32_t sy = (int32_t)(ih->mouse_y / c->scale) - (int32_t)c->origin_y;
                int32_t px = (int32_t)(ih->prev_x / c->scale) - (int32_t)c->origin_x;
                int32_t py = (int32_t)(ih->prev_y / c->scale) - (int32_t)c->origin_y;
                switch(ih->key_status[MOUSE_BUTTON_LEFT]) {
                    case KEY_JUST_PRESSED: {
                        canvas_begin_shape(c, sx, sy);
                        break;
                    }
                    case KEY_PRESSED: {
                        canvas_draw_empty_circle(c, sx, sy, px, py);
                        break;
                    }
                    case KEY_JUST_RELEASED: {
                        canvas_draw_filled_circle(c, sx, sy, px, py);
                        canvas_end_shape(c);
                        break;
                    }
                    default: break;
                }
                break;
            }
            default: break;
        }
    }
    // Need to set prev_x here so that it is based on the mouse position last frame not last event
    // (otherwise will get incomplete lines from Bresenham)
    ih->prev_x = ih->mouse_x;
    ih->prev_y = ih->mouse_y;
}

void button_state_manager_init(Button_State_Manager *bs) {
    // memset(bs->buttons, 0, sizeof(bs->buttons));
    bs->stb = BUTTON_NONE;
    bs->smb = BUTTON_NONE;
    bs->sbp = CLAY_STRING("S");
}

void update_sbp(Button_State_Manager *bs_manager, Button_Type t) {
    switch(t) {
        case BUTTON_LINE: bs_manager->sbp = CLAY_STRING("L"); break;
        case BUTTON_EMPTY_RECT: bs_manager->sbp = CLAY_STRING("ER"); break;
        case BUTTON_FILLED_RECT: bs_manager->sbp = CLAY_STRING("FR"); break;
        case BUTTON_EMPTY_CIRCLE: bs_manager->sbp = CLAY_STRING("EC"); break;
        case BUTTON_FILLED_CIRCLE: bs_manager->sbp = CLAY_STRING("FC"); break;
        default: bs_manager->sbp = CLAY_STRING("S"); break;
    }
}

