#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

typedef struct InputState {
    int quit;
    int any_key;
    int fire;
    int backspace;
    int submit;
    int next_field;
    int mouse_moved;
    int mouse_click;
    int mouse_x;
    int mouse_y;
    float angle_delta;
    float power_delta;
    char text[32];
    int text_len;
} InputState;

void input_begin_frame(InputState *input);
void input_handle_event(InputState *input, const SDL_Event *event);

#endif
