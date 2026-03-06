#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "game.h"
#include "graphics.h"
#include "input.h"

int main(void) {
    const int window_w = 960;
    const int window_h = 720;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Gorillas C Port",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_w, window_h,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    GraphicsContext gfx = {0};
    if (!graphics_init(&gfx, 24)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    GameState game;
    game_init(&game, window_w, window_h);
    SDL_StartTextInput();

    Uint64 prev_counter = SDL_GetPerformanceCounter();
    while (game.running) {
        Uint64 now_counter = SDL_GetPerformanceCounter();
        float dt_seconds = (float)(now_counter - prev_counter) / (float)SDL_GetPerformanceFrequency();
        prev_counter = now_counter;

        InputState input;
        input_begin_frame(&input);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            input_handle_event(&input, &event);
        }

        game_update(&game, &input, dt_seconds);
        graphics_render(renderer, &gfx, &game);
    }

    graphics_shutdown(&gfx);
    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
