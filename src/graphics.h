#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "game.h"

typedef struct GraphicsContext {
    TTF_Font *font;
} GraphicsContext;

int graphics_init(GraphicsContext *gfx, int font_px);
void graphics_shutdown(GraphicsContext *gfx);
void graphics_render(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game);

#endif
