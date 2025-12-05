#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>


// int main() {
//     printf("\033[2J");      // Clear screen
//     printf("\033[H");       // Move cursor to top-left

//     for (int i = 0; i < 10; i++) {
//         printf("Frame %d\n", i);
//         fflush(stdout);     // Ensure output is shown immediately
//         usleep(1000000);     // 0.5 second delay
//         printf("\033[H");   // Move cursor back to top-left
//     }

//     return 0;
// }


int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "SDL Text Example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load a font (make sure the path is correct)
    //TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 64);
    TTF_Font* font = TTF_OpenFont("../assets/fonts/Px437_IBM_VGA_8x16.ttf", 64);

//     char cwd[512];
// getcwd(cwd, sizeof(cwd));
// printf("Working dir: %s\n", cwd);

    if (!font) {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }



    char *intro_txt1 = "Copyright (C) CorpHackRyan Corporation 2025";
    char *intro_txt2 = "our mission is to hit your opponent with the exploding";
    char *intro_txt3 = "banana by varying the angle and power of your throw, taking";
    char *intro_txt4 = "into account wind speed, gravity, and the city skyline.";
    char *intro_txt5 = "The wind speed is shown by a directional arrow at the bottom";
    char *intro_txt6 = "of the playing field, its length relative to its strength.";
    char *intro_txt7 = "Press any key to continue";

    SDL_Color color = {255, 255, 0, 255}; // white


    SDL_Surface *surface = TTF_RenderText_Solid(font, intro_txt1, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    int running = 1;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255); // dark blue background
        SDL_RenderClear(renderer);

        // Draw the text
        SDL_Rect dst = {50, 50, 300, 50}; // position and size
        SDL_RenderCopy(renderer, texture, NULL, &dst);

        SDL_RenderPresent(renderer);
        
    }

    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}