#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"

int main(void) {
    const int window_w = 960;
    const int window_h = 720;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
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
    int sound_enabled = sound_init();
    int bgm_supported = sound_is_bgm_available();
    if (!sound_enabled) {
        fprintf(stderr, "Audio disabled: %s\n", SDL_GetError());
    }
    if (bgm_supported) {
        fprintf(stderr, "Background music support: ffplay found.\n");
    } else {
        fprintf(stderr, "Background music support: ffplay not found (game will run without BGM).\n");
    }
    if (sound_enabled) {
        sound_play_intro();
    }

    Uint64 prev_counter = SDL_GetPerformanceCounter();
    GamePhase last_phase = game.phase;
    int bgm_started = 0;
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
        if (bgm_supported && !bgm_started &&
            (game.phase == GAME_PHASE_AIMING || game.phase == GAME_PHASE_PROJECTILE || game.phase == GAME_PHASE_ROUND_END) &&
            (last_phase == GAME_PHASE_NAME_ENTRY || last_phase == GAME_PHASE_SPLASH)) {
            if (sound_start_bgm_loop()) {
                bgm_started = 1;
                fprintf(stderr, "Background music started.\n");
            } else {
                fprintf(stderr, "Background music disabled: could not start ffplay with embedded track.\n");
            }
        }
        graphics_render(renderer, &gfx, &game);
        last_phase = game.phase;
    }

    graphics_shutdown(&gfx);
    if (sound_enabled) {
        sound_shutdown();
    }
    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
