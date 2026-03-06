#include "input.h"

#include <string.h>

void input_begin_frame(InputState *input) {
    input->quit = 0;
    input->any_key = 0;
    input->fire = 0;
    input->backspace = 0;
    input->submit = 0;
    input->next_field = 0;
    input->angle_delta = 0.0f;
    input->power_delta = 0.0f;
    input->text[0] = '\0';
    input->text_len = 0;
}

void input_handle_event(InputState *input, const SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        input->quit = 1;
        return;
    }

    if (event->type == SDL_TEXTINPUT) {
        size_t copy_len = strlen(event->text.text);
        if (copy_len >= sizeof(input->text)) {
            copy_len = sizeof(input->text) - 1;
        }
        memcpy(input->text, event->text.text, copy_len);
        input->text[copy_len] = '\0';
        input->text_len = (int)copy_len;
        return;
    }

    if (event->type != SDL_KEYDOWN) {
        return;
    }

    input->any_key = 1;
    switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            input->quit = 1;
            break;
        case SDLK_SPACE:
        case SDLK_RETURN:
            input->fire = 1;
            input->submit = 1;
            break;
        case SDLK_TAB:
            input->next_field = 1;
            break;
        case SDLK_BACKSPACE:
            input->backspace = 1;
            break;
        case SDLK_UP:
            input->angle_delta = 1.0f;
            break;
        case SDLK_DOWN:
            input->angle_delta = -1.0f;
            break;
        case SDLK_RIGHT:
            input->power_delta = 2.5f;
            break;
        case SDLK_LEFT:
            input->power_delta = -2.5f;
            break;
        default:
            break;
    }
}
