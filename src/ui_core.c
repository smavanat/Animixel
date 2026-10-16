#include "ui_core.h"
#include "file_io.h"
#include "input.h"
#include "math_utils.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// TODO:
// Palette for now: https://lospec.com/palette-list/escorpion

//Clay Colours to be reused
const Clay_Color COLOUR_WHITE = (Clay_Color){255, 255, 255, 255};
const Clay_Color COLOUR_BLACK = (Clay_Color){0, 0, 0, 255};
const Clay_Color COLOUR_LIGHT = (Clay_Color){224, 215, 210, 255};
const Clay_Color COLOUR_RED = (Clay_Color){168, 66, 28, 255};
const Clay_Color COLOUR_ORANGE = (Clay_Color){255, 138, 50, 255};

//===..................................... UI STATE MANAGEMENT .......................................===
void set_toolbar_button_pressed_cb(Clay_ElementId id, Clay_PointerData info, void *userData) {
    if(info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Button_Type t = (intptr_t)userData;
        // Double clicking on a button should turn it off
        bs_manager.stb = (t == bs_manager.stb) ? BUTTON_NONE : t;

        update_sbp(&bs_manager, t);
    }
}

void set_menu_button_pressed_cb(Clay_ElementId id, Clay_PointerData info, void *userData) {
    if(info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Button_Type t = (intptr_t)userData;
        // Double clicking on a button should turn it off
        bs_manager.smb = (t == bs_manager.smb) ? BUTTON_NONE : t;
    }
}

void set_fe_button_pressed_cb(Clay_ElementId id, Clay_PointerData info, void *userData) {
    if(info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Button_Type t = (intptr_t)userData;
        // Double clicking on a button should turn it off
        bs_manager.smb = BUTTON_NONE;
        bs_manager.fe_mode = (t == BUTTON_SAVE_MENU) ? BUTTON_SAVE : BUTTON_LOAD;
        // bs_manager.smb = (t == bs_manager.smb) ? BUTTON_NONE : t;
        // bs_manager.show_fe = (bs_manager.smb == BUTTON_NONE) ? 0 : 1;
    }
}

void close_fe_cb(Clay_ElementId id, Clay_PointerData info, void *userData) {
    if(info.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Button_Type t = (intptr_t)userData;
        // Double clicking on a button should turn it off
        // bs_manager.smb = BUTTON_NONE;
        bs_manager.fe_mode = 0;
    }
}

//===..................................... CLAY WIDGET FUNCTIONS .......................................===

void create_static_button(Button_Type type, uint32_t width, uint32_t height, uint8_t padding, Clay_Color hover_colour,
                          Clay_Color basic_colour, Clay_String text, void (*button_handler)(Clay_ElementId id,
                          Clay_PointerData info, void *userData), void *userData) {
    uint8_t border = (bs_manager.stb == type) || (type == BUTTON_SHAPE &&
                     (bs_manager.stb == BUTTON_LINE || bs_manager.stb == BUTTON_EMPTY_RECT || bs_manager.stb == BUTTON_FILLED_RECT
                     || bs_manager.stb == BUTTON_EMPTY_CIRCLE || bs_manager.stb == BUTTON_FILLED_CIRCLE));

    CLAY(CLAY_IDI("Button", type), {.layout = {.sizing = {CLAY_SIZING_FIXED(width), CLAY_SIZING_FIXED(height)},
         .padding = CLAY_PADDING_ALL(padding), .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
         .backgroundColor = Clay_Hovered() ? hover_colour : basic_colour, .border = border
         ? (Clay_BorderElementConfig){COLOUR_BLACK, 2} : (Clay_BorderElementConfig){}}) {
        Clay_OnHover(button_handler, userData);

        CLAY_TEXT(text, {.textColor = COLOUR_BLACK, .textAlignment = CLAY_TEXT_ALIGN_CENTER});
    }
}

void create_file_explorer_row(uint32_t index, File_Explorer_Row *row, uint32_t height, uint8_t padding, Clay_Color hover_colour,
                              Clay_Color hover_border_colour, Clay_Color basic_colour, void (*button_handler)
                              (Clay_ElementId id, Clay_PointerData info, void *userData), void *userData) {
    CLAY(CLAY_IDI("Row", index), {.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(height)},
            .padding = CLAY_PADDING_ALL(padding), .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER},
         }, .backgroundColor = Clay_Hovered() ? hover_colour : basic_colour,
         .border = Clay_Hovered() ? (Clay_BorderElementConfig){hover_border_colour, 4} : (Clay_BorderElementConfig){}}){
        Clay_OnHover(button_handler, userData);

        Clay_String name = {.isStaticallyAllocated = true, .length = strlen(row->file_name), .chars = row->file_name};
        CLAY_TEXT(name, {.textColor = COLOUR_BLACK, .textAlignment = CLAY_TEXT_ALIGN_CENTER});
    }
}

//===..................................... CLAY MANAGEMENT FUNCTIONS .......................................===

/// Helper functions to handle errors from clay
void HandleClayErrors(Clay_ErrorData errorData) {
    //NOTE: See Clay_ErrorData struct for more info
    printf("%s\n", errorData.errorText.chars);
    switch (errorData.errorType) {
        //TODO:
    }
}

/// Callback function so that clay can measure text
/// TODO: Account for font sizes
Clay_Dimensions clay_measure_text_cb(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    BOB_Vector2 res;
    BOB_measure_char_string(text.chars, text.length, bitmap, &res);

    return (Clay_Dimensions){res.x, res.y};
}

void clay_init(size_t width, size_t height) {
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));

    Clay_Initialize(arena, (Clay_Dimensions){width, height}, (Clay_ErrorHandler){HandleClayErrors});
    Clay_SetMeasureTextFunction(clay_measure_text_cb, NULL);
}

void clay_update_dimensions(Input_Handler *ih, float dt, size_t width, size_t height) {
    //Update Clay internal layout dimensions to support resizing
    Clay_SetLayoutDimensions((Clay_Dimensions){width, height});
    Clay_SetPointerState((Clay_Vector2){ih->mouse_x, ih->mouse_y}, (ih->key_status[MOUSE_BUTTON_LEFT] == KEY_JUST_PRESSED) || (ih->key_status[MOUSE_BUTTON_LEFT] == KEY_PRESSED));
    Clay_UpdateScrollContainers(true, (Clay_Vector2){ih->scroll_x, ih->scroll_y}, dt);
}

// TODO: Fix z=index rendering
void clay_render(BOB_Renderer_Handle r, Clay_RenderCommandArray renderCommands) {
    for(int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand *renderCommand = &renderCommands.internalArray[i];

        switch(renderCommand->commandType) {
            // This command type should be skipped.
            case CLAY_RENDER_COMMAND_TYPE_NONE:
            break;
            // The renderer should draw a solid color rectangle.
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
                //TODO: Implement rounded corners
                BOB_draw_quad(r, (BOB_Quad){
                    renderCommand->boundingBox.x,
                    renderCommand->boundingBox.y,
                    renderCommand->boundingBox.width,
                    renderCommand->boundingBox.height,
                }, (BOB_Vector4){
                    renderCommand->renderData.rectangle.backgroundColor.r/255.0f,
                    renderCommand->renderData.rectangle.backgroundColor.g/255.0f,
                    renderCommand->renderData.rectangle.backgroundColor.b/255.0f,
                    renderCommand->renderData.rectangle.backgroundColor.a/255.0f,
                }, renderCommand->zIndex + 10, 0.0f);
            break;
            // The renderer should draw a colored border inset into the bounding box.
            case CLAY_RENDER_COMMAND_TYPE_BORDER:
                // For some reason have to set the thickness to be the left border side thickness otherwise it doesn't work???
                BOB_draw_line(r, (BOB_Vector2){renderCommand->boundingBox.x, renderCommand->boundingBox.y}, (BOB_Vector2){renderCommand->boundingBox.x +
                    renderCommand->boundingBox.width, renderCommand->boundingBox.y}, renderCommand->renderData.border.width.left,
                    (BOB_Vector4){
                        renderCommand->renderData.border.color.r/255.0f,
                        renderCommand->renderData.border.color.b/255.0f,
                        renderCommand->renderData.border.color.g/255.0f,
                        renderCommand->renderData.border.color.a/255.0f,
                    }, renderCommand->zIndex + 50);
                BOB_draw_line(r, (BOB_Vector2){renderCommand->boundingBox.x, renderCommand->boundingBox.y}, (BOB_Vector2){renderCommand->boundingBox.x,
                    renderCommand->boundingBox.y + renderCommand->boundingBox.height}, renderCommand->renderData.border.width.left,
                    (BOB_Vector4){
                        renderCommand->renderData.border.color.r/255.0f,
                        renderCommand->renderData.border.color.b/255.0f,
                        renderCommand->renderData.border.color.g/255.0f,
                        renderCommand->renderData.border.color.a/255.0f,
                    }, renderCommand->zIndex + 50);
                BOB_draw_line(r, (BOB_Vector2){renderCommand->boundingBox.x + renderCommand->boundingBox.width, renderCommand->boundingBox.y},
                    (BOB_Vector2){renderCommand->boundingBox.x + renderCommand->boundingBox.width, renderCommand->boundingBox.y + renderCommand->boundingBox.height},
                    renderCommand->renderData.border.width.left, (BOB_Vector4){
                        renderCommand->renderData.border.color.r/255.0f,
                        renderCommand->renderData.border.color.b/255.0f,
                        renderCommand->renderData.border.color.g/255.0f,
                        renderCommand->renderData.border.color.a/255.0f,
                    }, renderCommand->zIndex + 50);
                BOB_draw_line(r, (BOB_Vector2){renderCommand->boundingBox.x, renderCommand->boundingBox.y + renderCommand->boundingBox.height},
                    (BOB_Vector2){renderCommand->boundingBox.x + renderCommand->boundingBox.width, renderCommand->boundingBox.y + renderCommand->boundingBox.height},
                    renderCommand->renderData.border.width.left, (BOB_Vector4){
                        renderCommand->renderData.border.color.r/255.0f,
                        renderCommand->renderData.border.color.b/255.0f,
                        renderCommand->renderData.border.color.g/255.0f,
                        renderCommand->renderData.border.color.a/255.0f,
                    }, renderCommand->zIndex + 50);
            break;
            // The renderer should draw text.
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData data = renderCommand->renderData.text;
                uint16_t fontSize = (data.fontSize > 0) ? data.fontSize : 20;
                BOB_Vector2 start_pos = {renderCommand->boundingBox.x, renderCommand->boundingBox.y};
                BOB_draw_char_string(bitmap, data.stringContents.chars, data.stringContents.length, &start_pos,
                                   (BOB_Vector4) { data.textColor.r/255.0f, data.textColor.g/255.0f, data.textColor.b/255.0f, data.textColor.a/255.0f, }, renderCommand->zIndex + 20);
            }
            break;
            // The renderer should draw an image.
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:
                printf("Image Rendering not currently implemented\n");
            break;
            // The renderer should begin clipping all future draw commands, only rendering content that falls within the provided boundingBox.
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                BOB_Clip_Dir dir;
                if(renderCommand->renderData.clip.vertical && renderCommand->renderData.clip.horizontal) dir = BOB_CLIP_BOTH;
                else if(renderCommand->renderData.clip.vertical) dir = BOB_CLIP_VERT;
                else dir = BOB_CLIP_HORZ;
                BOB_start_clip(r, (BOB_Quad){renderCommand->boundingBox.x, renderCommand->boundingBox.y, renderCommand->boundingBox.width, renderCommand->boundingBox.height}, dir);
            }
            break;
            // The renderer should finish any previously active clipping, and begin rendering elements in full again.
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                BOB_end_clip(r);
            break;
            // The renderer should begin performing a "color overlay" on all subsequent render commands until disabled again.
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START:
                printf("Colour Overlay not currently implemented\n");
            break;
            // The renderer should disable any previously active "color overlay" and render elements with their standard colors again.
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END:
                printf("Colour Overlay not currently implemented\n");
            break;
            // The renderer should provide a custom implementation for handling this render command based on its .customData
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Custom_Element_Data *custom_elem = (Custom_Element_Data *)renderCommand->renderData.custom.customData;
                if(!custom_elem) continue;
                switch(custom_elem->type){
                    case CUSTOM_ELEMENT_TYPE_CANVAS: {
                        custom_elem->canvas->origin_x = clampi(((int32_t)renderCommand->boundingBox.x / custom_elem->canvas->scale) - (custom_elem->canvas->width / 2), 0, INT32_MAX);
                        custom_elem->canvas->origin_y = clampi(((int32_t)renderCommand->boundingBox.y / custom_elem->canvas->scale) - (custom_elem->canvas->height / 2), 0, INT32_MAX);
                        canvas_draw(custom_elem->canvas, r);
                        // Draw a quad around the canvas to show it's border
                        BOB_draw_unfilled_quad(r, (BOB_Quad){(custom_elem->canvas->origin_x * custom_elem->canvas->scale) - 3,
                                                             (custom_elem->canvas->origin_y * custom_elem->canvas->scale) - 3,
                                                             (custom_elem->canvas->width * custom_elem->canvas->scale) + 7,
                                                             (custom_elem->canvas->width * custom_elem->canvas->scale) + 7},
                                                             5.0f, (BOB_Vector4){0, 0, 0, 1}, renderCommand->zIndex + 10, 0.0f);
                    }
                }
            }
            break;
        }
    }
}

void clay_free() {
}

//===..................................... CLAY LAYOUT .......................................===
Clay_RenderCommandArray editor_layout(float dt, Custom_Element_Data *canvas, File_Explorer *fe) {
    // All clay layouts are declared between Clay_BeginLayout and Clay_EndLayout
    Clay_BeginLayout();

    CLAY(CLAY_ID("OuterContainer"), {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {
        CLAY(CLAY_ID("MenuBar"), {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(44)},
            .padding = CLAY_PADDING_ALL(4), .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}, .backgroundColor = COLOUR_RED}) {
            create_static_button(BUTTON_FILE, 120, 36, 8, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("File"), set_menu_button_pressed_cb, (void *)BUTTON_FILE);
        }
        CLAY(CLAY_ID("Editor"), {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {
            CLAY(CLAY_ID("CanvasContainer"), {.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}, .backgroundColor = COLOUR_LIGHT}) {
                CLAY(CLAY_ID("Canvas"), {.custom = {.customData = canvas}}) {}
            }
            CLAY(CLAY_ID("Buttons"), {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = {CLAY_SIZING_GROW(0, 44), CLAY_SIZING_GROW(0)},
                .childGap = 8, .padding = CLAY_PADDING_ALL(4), .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_CENTER}}, .backgroundColor = COLOUR_RED}) {
                create_static_button(BUTTON_DRAW, 36, 36, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("D"), set_toolbar_button_pressed_cb, (void *)BUTTON_DRAW);
                create_static_button(BUTTON_ERASE, 36, 36, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("E"), set_toolbar_button_pressed_cb, (void *)BUTTON_ERASE);
                create_static_button(BUTTON_SHAPE, 36, 36, 4, COLOUR_LIGHT, COLOUR_ORANGE, bs_manager.sbp, set_toolbar_button_pressed_cb, (void *)BUTTON_SHAPE);
            }

        }
    }

    // Generates floating toolbar holding all of the shape options
    // if the shape button is pressed
    if(bs_manager.stb == BUTTON_SHAPE) {
        CLAY(CLAY_ID("ShapeOptions"),{.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 2, .padding = CLAY_PADDING_ALL(2)},
             .floating = {.parentId = CLAY_IDI("Button", BUTTON_SHAPE).id, .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID, .zIndex = 20,
             .attachPoints = {.element = CLAY_ATTACH_POINT_RIGHT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER}}, .backgroundColor = COLOUR_RED}) {
            create_static_button(BUTTON_LINE, 32, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("L"), set_toolbar_button_pressed_cb, (void *)BUTTON_LINE);
            create_static_button(BUTTON_EMPTY_RECT, 32, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("ER"), set_toolbar_button_pressed_cb, (void *)BUTTON_EMPTY_RECT);
            create_static_button(BUTTON_FILLED_RECT, 32, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("FR"), set_toolbar_button_pressed_cb, (void *)BUTTON_FILLED_RECT);
            create_static_button(BUTTON_EMPTY_CIRCLE, 32, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("EC"), set_toolbar_button_pressed_cb, (void *)BUTTON_EMPTY_CIRCLE);
            create_static_button(BUTTON_FILLED_CIRCLE, 32, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("FC"), set_toolbar_button_pressed_cb, (void *)BUTTON_FILLED_CIRCLE);
        }
    }

    if(bs_manager.smb == BUTTON_FILE) {
        CLAY(CLAY_ID("FileMenu"),{.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,},
             .floating = {.parentId = CLAY_IDI("Button", BUTTON_FILE).id, .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID, .zIndex = 20,
             .attachPoints = {.element = CLAY_ATTACH_POINT_LEFT_TOP, .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM}}, .backgroundColor = COLOUR_RED}) {
            create_static_button(BUTTON_SAVE_MENU, 240, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("SAVE"), set_fe_button_pressed_cb, (void *)BUTTON_SAVE_MENU);
            create_static_button(BUTTON_LOAD_MENU, 240, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("LOAD"), set_fe_button_pressed_cb, (void *)BUTTON_LOAD_MENU);
        }
    }

    if(bs_manager.fe_mode) {
        // TODO: Make this scale relative to the screen size
        CLAY(CLAY_ID("FileExplorer"), {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = {CLAY_SIZING_FIXED(512), CLAY_SIZING_FIXED(512)},
            .childGap = 2, .padding = CLAY_PADDING_ALL(2), .childAlignment = CLAY_ALIGN_X_CENTER}, .floating = {.parentId = CLAY_ID("OuterContainer").id, 
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID, .zIndex = 30, .attachPoints = {.element = CLAY_ATTACH_POINT_CENTER_CENTER,
            .parent = CLAY_ATTACH_POINT_CENTER_CENTER}},.border = {COLOUR_BLACK, 4}, .backgroundColor = COLOUR_RED}) {
            CLAY(CLAY_ID("FileExplorerHeader"), {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .sizing = {CLAY_SIZING_GROW(0)},
             .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}, .padding = CLAY_PADDING_ALL(2)}, .backgroundColor = COLOUR_WHITE}) {
                CLAY_TEXT(CLAY_STRING("File Explorer"), {.textColor = COLOUR_BLACK, .textAlignment = CLAY_TEXT_ALIGN_CENTER});
                CLAY(CLAY_ID("HeaderBuffer"), {.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {}
                create_static_button(BUTTON_CLOSE, 24, 24, 1, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("x"), close_fe_cb, NULL);
            }
            CLAY(CLAY_ID("FileExplorerMain"), {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .childGap = 16, .padding = CLAY_PADDING_ALL(16), .childAlignment = CLAY_ALIGN_X_CENTER},}) {
                CLAY(CLAY_ID("Rows"), {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}},
                    .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()}, .backgroundColor = COLOUR_LIGHT}) {
                    // Create the file rows
                    for(size_t i = 0; i < fe->num_rows; i++) {
                        create_file_explorer_row(i, &fe->rows[i], 20, 2, COLOUR_WHITE, COLOUR_ORANGE, COLOUR_LIGHT, NULL, NULL);
                    }
                }
            }
            CLAY(CLAY_ID("FileExplorerButtons"), {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT, .sizing = {CLAY_SIZING_GROW(0)},
            .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}, .padding = CLAY_PADDING_ALL(8)}}) {
                create_static_button(BUTTON_DELTE, 128, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("Delete"), NULL, NULL);
                CLAY(CLAY_ID("ButtonBuffer"), {.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {}
                if(bs_manager.fe_mode == BUTTON_SAVE)
                    create_static_button(BUTTON_SAVE, 128, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("Save"), NULL, NULL);
                else
                    create_static_button(BUTTON_LOAD, 128, 32, 4, COLOUR_LIGHT, COLOUR_ORANGE, CLAY_STRING("Load"), NULL, NULL);
            }
        }
    }
    // All clay layouts are declared between Clay_BeginLayout and Clay_EndLayout
    return Clay_EndLayout(dt); // deltaTime is the time since the last frame, and is used for transitions
}
