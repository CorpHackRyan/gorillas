#include "graphics.h"

#include <math.h>
#include <stdio.h>

typedef enum GorillaArms {
    GORILLA_ARMS_DOWN = 0,
    GORILLA_LEFT_UP,
    GORILLA_RIGHT_UP
} GorillaArms;

enum {
    GORILLA_W = 16,
    GORILLA_H = 16
};

static const char *GORILLA_SPRITE_DOWN[GORILLA_H] = {
    "......BBBB......",
    ".....BBBBBB.....",
    "....BBBBBBBB....",
    "...BBBKBBKBBB...",
    "...BBBBBBBBBB...",
    "..BBBBBBBBBBBB..",
    ".BBBBBBBBBBBBBB.",
    "BBBBBBBBBBBBBBBB",
    "BBBBBBBBBBBBBBBB",
    ".BBBBBBBBBBBBBB.",
    "..BBBBBBBBBBBB..",
    "..BBBB..BBBB....",
    "..BBBB..BBBB....",
    "..BBB....BBB....",
    ".BBBB....BBBB...",
    "BBBB......BBBB.."
};

static const char *GORILLA_SPRITE_LEFT_UP[GORILLA_H] = {
    ".BB...BBBB......",
    "BBB..BBBBBB.....",
    "BBB.BBBBBBBB....",
    "BBBBBBBKBBKBBB..",
    ".BBBBBBBBBBBBB..",
    "..BBBBBBBBBBBB..",
    "...BBBBBBBBBBBB.",
    "...BBBBBBBBBBBBB",
    "...BBBBBBBBBBBBB",
    "...BBBBBBBBBBBB.",
    "...BBBBBBBBBBB..",
    "..BBBB....BBBB..",
    "..BBBB....BBBB..",
    "..BBB......BBB..",
    ".BBBB......BBBB.",
    "BBBB........BBBB"
};

static const char *GORILLA_SPRITE_RIGHT_UP[GORILLA_H] = {
    "......BBBB...BB.",
    ".....BBBBBB..BBB",
    "....BBBBBBBB.BBB",
    "..BBBKBBKBBBBBBB",
    "..BBBBBBBBBBBBB.",
    "..BBBBBBBBBBBB..",
    ".BBBBBBBBBBBB...",
    "BBBBBBBBBBBBB...",
    "BBBBBBBBBBBBB...",
    ".BBBBBBBBBBBB...",
    "..BBBBBBBBBBB...",
    "..BBBB....BBBB..",
    "..BBBB....BBBB..",
    "..BBB......BBB..",
    ".BBBB......BBBB.",
    "BBBB........BBBB"
};

enum {
    BANANA_W = 7,
    BANANA_H = 7
};

/* 4 frames: left, up, down, right (matching the BAS rotation order). */
static const char *BANANA_FRAMES[4][BANANA_H] = {
    {
        "...YYY.",
        "..YYYYY",
        ".YY....",
        "YY.....",
        ".YY....",
        "..YYYYY",
        "...YYY."
    },
    {
        "..YYY..",
        ".Y...Y.",
        "Y.....Y",
        "Y.....Y",
        ".Y...Y.",
        "..YYY..",
        "...Y..."
    },
    {
        "...Y...",
        "..YYY..",
        ".Y...Y.",
        "Y.....Y",
        "Y.....Y",
        ".Y...Y.",
        "..YYY.."
    },
    {
        ".YYY...",
        "YYYYY..",
        "....YY.",
        ".....YY",
        "....YY.",
        "YYYYY..",
        ".YYY..."
    }
};

static void draw_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

static void draw_filled_rect(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

static unsigned int hash_u32(unsigned int x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static int window_is_lit(const BuildingState *b, int row, int col) {
    unsigned int key = (unsigned int)(b->x * 73856093) ^ (unsigned int)(b->top_y * 19349663) ^
                       (unsigned int)(row * 83492791) ^ (unsigned int)(col * 2654435761U);
    return (hash_u32(key) % 100U) < 35U;
}

static void draw_building_windows(SDL_Renderer *renderer, const BuildingState *b, int street_y) {
    const int win_w = 6;
    const int win_h = 8;
    const int gap_x = 6;
    const int gap_y = 8;
    SDL_Color lit = {255, 230, 120, 255};
    SDL_Color dark = {25, 25, 40, 255};

    int x_start = b->x + 5;
    int x_end = b->x + b->width - win_w - 5;
    int y_start = b->top_y + 8;
    int y_end = street_y - win_h - 6;
    int row = 0;

    for (int y = y_start; y <= y_end; y += gap_y) {
        int col = 0;
        for (int x = x_start; x <= x_end; x += gap_x) {
            SDL_Color c = window_is_lit(b, row, col) ? lit : dark;
            draw_filled_rect(renderer, x, y, win_w, win_h, c);
            col++;
        }
        row++;
    }
}

static void draw_banana(SDL_Renderer *renderer, int center_x, int center_y, int frame, int screen_w) {
    int scale = (screen_w >= 800) ? 2 : 1;
    int left = center_x - ((BANANA_W * scale) / 2);
    int top = center_y - ((BANANA_H * scale) / 2);
    SDL_Color body = {255, 232, 56, 255};
    SDL_Color tip = {38, 24, 8, 255};
    const char *const *sprite = BANANA_FRAMES[frame & 3];

    for (int row = 0; row < BANANA_H; row++) {
        for (int col = 0; col < BANANA_W; col++) {
            char pixel = sprite[row][col];
            if (pixel == '.') {
                continue;
            }
            draw_filled_rect(
                renderer,
                left + col * scale,
                top + row * scale,
                scale,
                scale,
                (pixel == 'Y') ? body : tip
            );
        }
    }
}

static void draw_sun(
    SDL_Renderer *renderer,
    int center_x,
    int center_y,
    int shocked,
    int track_enabled,
    float target_x,
    float target_y
) {
    SDL_Color sun = {255, 255, 0, 255};
    SDL_Color feature = {0, 0, 0, 255};
    float dir_x = 0.0f;
    float dir_y = 0.0f;
    float len = 1.0f;
    int eye_shift_x = 0;
    int eye_shift_y = 0;
    int mouth_shift_x = 0;
    int mouth_shift_y = 0;

    if (track_enabled) {
        dir_x = target_x - (float)center_x;
        dir_y = target_y - (float)center_y;
        len = sqrtf(dir_x * dir_x + dir_y * dir_y);
        if (len > 0.001f) {
            dir_x /= len;
            dir_y /= len;
        }
        eye_shift_x = (int)(dir_x * 3.0f);
        eye_shift_y = (int)(dir_y * 2.0f);
        mouth_shift_x = (int)(dir_x * 2.0f);
        mouth_shift_y = (int)(dir_y * 2.0f);
    }

    /* Rays */
    SDL_SetRenderDrawColor(renderer, sun.r, sun.g, sun.b, sun.a);
    for (int i = -1; i <= 1; i++) {
        SDL_RenderDrawLine(renderer, center_x - 28, center_y + i, center_x - 40, center_y + i);
        SDL_RenderDrawLine(renderer, center_x + 28, center_y + i, center_x + 40, center_y + i);
        SDL_RenderDrawLine(renderer, center_x + i, center_y - 28, center_x + i, center_y - 40);
        SDL_RenderDrawLine(renderer, center_x + i, center_y + 28, center_x + i, center_y + 40);
    }
    SDL_RenderDrawLine(renderer, center_x - 22, center_y - 22, center_x - 31, center_y - 31);
    SDL_RenderDrawLine(renderer, center_x - 21, center_y - 22, center_x - 30, center_y - 31);
    SDL_RenderDrawLine(renderer, center_x + 22, center_y - 22, center_x + 31, center_y - 31);
    SDL_RenderDrawLine(renderer, center_x + 21, center_y - 22, center_x + 30, center_y - 31);
    SDL_RenderDrawLine(renderer, center_x - 22, center_y + 22, center_x - 31, center_y + 31);
    SDL_RenderDrawLine(renderer, center_x - 21, center_y + 22, center_x - 30, center_y + 31);
    SDL_RenderDrawLine(renderer, center_x + 22, center_y + 22, center_x + 31, center_y + 31);
    SDL_RenderDrawLine(renderer, center_x + 21, center_y + 22, center_x + 30, center_y + 31);

    /* Face */
    draw_filled_circle(renderer, center_x, center_y, 24, sun);
    draw_filled_circle(renderer, center_x - 9, center_y - 6, 4, feature);
    draw_filled_circle(renderer, center_x + 9, center_y - 6, 4, feature);
    draw_filled_circle(renderer, center_x - 9 + eye_shift_x, center_y - 6 + eye_shift_y, 2, sun);
    draw_filled_circle(renderer, center_x + 9 + eye_shift_x, center_y - 6 + eye_shift_y, 2, sun);
    draw_filled_circle(renderer, center_x - 9 + eye_shift_x, center_y - 6 + eye_shift_y, 1, feature);
    draw_filled_circle(renderer, center_x + 9 + eye_shift_x, center_y - 6 + eye_shift_y, 1, feature);
    SDL_SetRenderDrawColor(renderer, feature.r, feature.g, feature.b, feature.a);
    if (shocked) {
        draw_filled_circle(renderer, center_x + mouth_shift_x, center_y + mouth_shift_y + 14, 4, feature);
    } else {
        for (int x = -10; x <= 10; x++) {
            int y = 6 - (x * x) / 20;
            SDL_RenderDrawPoint(renderer, center_x + mouth_shift_x + x, center_y + mouth_shift_y + 12 + y);
            SDL_RenderDrawPoint(renderer, center_x + mouth_shift_x + x, center_y + mouth_shift_y + 13 + y);
        }
    }
}

static void draw_gorilla(SDL_Renderer *renderer, int x_center, int roof_y, GorillaArms arms) {
    const int scale = 3;
    const char *const *frame = GORILLA_SPRITE_DOWN;
    int x_left = x_center - (GORILLA_W * scale) / 2;
    int y_top = roof_y - GORILLA_H * scale;
    SDL_Color body = {196, 132, 52, 255};
    SDL_Color detail = {18, 10, 3, 255};

    if (arms == GORILLA_LEFT_UP) {
        frame = GORILLA_SPRITE_LEFT_UP;
    } else if (arms == GORILLA_RIGHT_UP) {
        frame = GORILLA_SPRITE_RIGHT_UP;
    }

    for (int row = 0; row < GORILLA_H; row++) {
        for (int col = 0; col < GORILLA_W; col++) {
            char pixel = frame[row][col];
            if (pixel == '.') {
                continue;
            }
            if (pixel == 'B') {
                draw_filled_rect(
                    renderer,
                    x_left + col * scale,
                    y_top + row * scale,
                    scale,
                    scale,
                    body
                );
            } else if (pixel == 'K') {
                draw_filled_rect(
                    renderer,
                    x_left + col * scale,
                    y_top + row * scale,
                    scale,
                    scale,
                    detail
                );
            }
        }
    }
}

static TTF_Font *load_font_with_fallback(int font_px) {
    const char *font_paths[] = {
        "assets/fonts/Px437_IBM_VGA_8x16.ttf",
        "../assets/fonts/Px437_IBM_VGA_8x16.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationMono-Regular.ttf"
    };

    for (size_t i = 0; i < sizeof(font_paths) / sizeof(font_paths[0]); i++) {
        TTF_Font *font = TTF_OpenFont(font_paths[i], font_px);
        if (font) {
            if (i > 1) {
                fprintf(stderr, "Using fallback system font: %s\n", font_paths[i]);
            }
            return font;
        }
    }

    fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
    fprintf(stderr, "Tried font paths:\n");
    for (size_t i = 0; i < sizeof(font_paths) / sizeof(font_paths[0]); i++) {
        fprintf(stderr, "  - %s\n", font_paths[i]);
    }
    return NULL;
}

static void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}

static void draw_text_center(SDL_Renderer *renderer, TTF_Font *font, const char *text, int y, SDL_Color color, int screen_w) {
    int text_w = 0;
    int text_h = 0;
    if (TTF_SizeText(font, text, &text_w, &text_h) != 0) {
        return;
    }
    draw_text(renderer, font, text, (screen_w - text_w) / 2, y, color);
}

int graphics_init(GraphicsContext *gfx, int font_px) {
    gfx->font = load_font_with_fallback(font_px);
    return gfx->font ? 1 : 0;
}

void graphics_shutdown(GraphicsContext *gfx) {
    if (gfx->font) {
        TTF_CloseFont(gfx->font);
        gfx->font = NULL;
    }
}

static void render_sparkle_border(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game) {
    Uint32 ticks = SDL_GetTicks();
    int phase = (int)((ticks / 140U) % 5U);
    int spacing = 24;
    int margin = 8;
    SDL_Color sparkle = {255, 64, 64, 255};

    for (int x = margin, i = 0; x < game->screen_w - margin; x += spacing, i++) {
        if (((i + phase) % 5) == 0) {
            draw_text(renderer, gfx->font, "*", x, margin, sparkle);
            draw_text(renderer, gfx->font, "*", x, game->screen_h - 40, sparkle);
        }
    }

    for (int y = margin + 30, i = 0; y < game->screen_h - 40; y += spacing, i++) {
        if (((i + phase) % 5) == 0) {
            draw_text(renderer, gfx->font, "*", margin, y, sparkle);
            draw_text(renderer, gfx->font, "*", game->screen_w - 24, game->screen_h - y - 24, sparkle);
        }
    }
}

static void render_splash(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {170, 170, 170, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    const char *intro_lines[] = {
        "Your mission is to hit your opponent with the exploding",
        "banana by varying the angle and power of your throw, taking",
        "into account wind speed, gravity, and the city skyline.",
        "The wind speed is shown by a directional arrow at the bottom",
        "of the playing field, its length relative to its strength."
    };

    render_sparkle_border(renderer, gfx, game);
    draw_text_center(renderer, gfx->font, "Q B a s i c    G O R I L L A S", 120, white, game->screen_w);
    draw_text_center(renderer, gfx->font, "Copyright (C) Microsoft Corporation 1990", 170, gray, game->screen_w);
    draw_text_center(renderer, gfx->font, intro_lines[0], 240, gray, game->screen_w);
    draw_text_center(renderer, gfx->font, intro_lines[1], 275, gray, game->screen_w);
    draw_text_center(renderer, gfx->font, intro_lines[2], 310, gray, game->screen_w);
    draw_text_center(renderer, gfx->font, intro_lines[3], 345, gray, game->screen_w);
    draw_text_center(renderer, gfx->font, intro_lines[4], 380, gray, game->screen_w);
    draw_text_center(
        renderer,
        gfx->font,
        "Press any key to continue",
        game->screen_h - 90,
        yellow,
        game->screen_w
    );
}

static void render_name_entry(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game) {
    SDL_Color white = {230, 230, 230, 255};
    SDL_Color gray = {150, 150, 150, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    char line1[128];
    char line2[128];
    char line3[128];
    char line4[128];
    char line5[128];
    const char *cursor = "_";

    snprintf(line1, sizeof(line1), "Name of Player 1 (Default = 'Player 1'): %s%s",
             game->player_names[0], (game->active_name_index == 0) ? cursor : "");
    snprintf(line2, sizeof(line2), "Name of Player 2 (Default = 'Player 2'): %s%s",
             game->player_names[1], (game->active_name_index == 1) ? cursor : "");
    snprintf(
        line3,
        sizeof(line3),
        "Play to how many total points (Default = 3): %s%s",
        game->points_input,
        (game->active_name_index == 2) ? cursor : ""
    );
    snprintf(
        line4,
        sizeof(line4),
        "Gravity in Meters/Sec (Earth = 9.8): %s%s",
        game->gravity_input,
        (game->active_name_index == 3) ? cursor : ""
    );
    snprintf(
        line5,
        sizeof(line5),
        "Mouse Aim (On/Off, default On): %s%s",
        game->mouse_aim_input,
        (game->active_name_index == 4) ? cursor : ""
    );

    draw_text_center(renderer, gfx->font, "Enter Setup Values", 140, white, game->screen_w);
    draw_text(renderer, gfx->font, line1, 80, 260, (game->active_name_index == 0) ? yellow : white);
    draw_text(renderer, gfx->font, line2, 80, 320, (game->active_name_index == 1) ? yellow : white);
    draw_text(renderer, gfx->font, line3, 80, 380, (game->active_name_index == 2) ? yellow : white);
    draw_text(renderer, gfx->font, line4, 80, 440, (game->active_name_index == 3) ? yellow : white);
    draw_text(renderer, gfx->font, line5, 80, 500, (game->active_name_index == 4) ? yellow : white);
    draw_text(renderer, gfx->font, "Press Enter to confirm each field", 80, 560, gray);
    draw_text(renderer, gfx->font, "Leave points/gravity/mouse blank for defaults", 80, 595, gray);
}

static void render_game_over(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color cyan = {0, 255, 255, 255};
    SDL_Color orange = {255, 170, 60, 255};
    char score_line1[96];
    char score_line2[96];
    char winner_line[96];
    Uint32 ticks = SDL_GetTicks();
    int dance_count = 6;
    int base_y = game->screen_h - 95;
    int spacing = game->screen_w / (dance_count + 1);
    int banner_w = 0;
    int banner_h = 0;
    const char *banner = "WINNER!";
    int banner_track = 0;
    int banner_x = 0;
    int banner_y = 40;

    snprintf(score_line1, sizeof(score_line1), "%s: %d", game->player_names[0], game->scores[0]);
    snprintf(score_line2, sizeof(score_line2), "%s: %d", game->player_names[1], game->scores[1]);
    if (game->match_winner_index >= 0) {
        snprintf(winner_line, sizeof(winner_line), "%s wins the match!", game->player_names[game->match_winner_index]);
    } else {
        snprintf(winner_line, sizeof(winner_line), "Match complete");
    }

    if (TTF_SizeText(gfx->font, banner, &banner_w, &banner_h) == 0) {
        banner_track = game->screen_w + banner_w + 40;
        if (banner_track > 0) {
            banner_x = (int)((ticks / 6U) % (unsigned int)banner_track) - banner_w - 20;
        }
        draw_text(renderer, gfx->font, banner, banner_x + 2, banner_y + 2, orange);
        draw_text(renderer, gfx->font, banner, banner_x, banner_y, yellow);
    } else {
        draw_text_center(renderer, gfx->font, banner, banner_y, yellow, game->screen_w);
    }

    for (int i = 0; i < dance_count; i++) {
        int x = spacing * (i + 1);
        int bob = (int)(sinf(((float)ticks * 0.008f) + (float)i) * 7.0f);
        int arm_flip = ((int)(ticks / 120U) + i) & 1;
        GorillaArms arms = arm_flip ? GORILLA_LEFT_UP : GORILLA_RIGHT_UP;
        draw_gorilla(renderer, x, base_y + bob, arms);
    }

    draw_text_center(renderer, gfx->font, "GAME OVER!", 170, white, game->screen_w);
    draw_text_center(renderer, gfx->font, "Score:", 260, white, game->screen_w);
    draw_text_center(renderer, gfx->font, score_line1, 310, yellow, game->screen_w);
    draw_text_center(renderer, gfx->font, score_line2, 350, yellow, game->screen_w);
    draw_text_center(renderer, gfx->font, winner_line, 430, cyan, game->screen_w);
    draw_text_center(renderer, gfx->font, "Press any key to continue", game->screen_h - 40, white, game->screen_w);
}

static void render_mouse_aim_preview(SDL_Renderer *renderer, const GameState *game, const PlayerState *player, int player_index) {
    const float y_scale = (float)game->screen_h / 350.0f;
    const float wind_accel = game->wind.accel_x / 5.0f;
    const float gravity = game->gravity_mps2;
    float angle_rad = player->angle_deg * (float)(3.14159265358979323846 / 180.0);
    float dir = (player_index == 0) ? 1.0f : -1.0f;
    float vx = cosf(angle_rad) * player->power * dir;
    float vy = -sinf(angle_rad) * player->power;
    float x = player->x;
    float y = player->y - 24.0f;
    SDL_Color c = {255, 255, 0, 255};

    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    for (int i = 0; i < 10; i++) {
        float t = 0.25f + (float)i * 0.22f;
        float px = x + (vx * t) + (0.5f * wind_accel * t * t);
        float py = y + ((vy * t) + (0.5f * gravity * t * t)) * y_scale;
        SDL_Rect dash = {(int)px - 4, (int)py - 1, 8, 2};
        SDL_RenderFillRect(renderer, &dash);
    }
}

static void render_gameplay(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color cyan = {0, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color sky = {0, 0, 128, 255};
    SDL_Color red = {255, 80, 60, 255};
    int p2_name_w = 0;
    int p2_name_h = 0;

    draw_sun(
        renderer,
        game->screen_w / 2,
        70,
        game->sun_shocked,
        game->projectile.active,
        game->projectile.x,
        game->projectile.y
    );

    for (int i = 0; i < game->building_count; i++) {
        const BuildingState *b = &game->buildings[i];
        SDL_Rect building = {b->x, b->top_y, b->width, game->street_y - b->top_y};
        SDL_SetRenderDrawColor(renderer, b->r, b->g, b->b, 255);
        SDL_RenderFillRect(renderer, &building);
        draw_building_windows(renderer, b, game->street_y);
    }

    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_Rect street = {0, game->street_y, game->screen_w, game->screen_h - game->street_y};
    SDL_RenderFillRect(renderer, &street);

    for (int i = 0; i < MAX_CRATERS; i++) {
        if (game->craters[i].active) {
            draw_filled_circle(
                renderer,
                (int)game->craters[i].x,
                (int)game->craters[i].y,
                (int)game->craters[i].radius,
                sky
            );
        }
    }

    GorillaArms p1_arms = GORILLA_ARMS_DOWN;
    GorillaArms p2_arms = GORILLA_ARMS_DOWN;
    if (game->phase == GAME_PHASE_PROJECTILE) {
        if (game->current_player == 0) {
            p1_arms = GORILLA_RIGHT_UP;
        } else {
            p2_arms = GORILLA_LEFT_UP;
        }
    } else if (game->phase == GAME_PHASE_ROUND_END && game->round_winner_index >= 0) {
        int dance_up = ((SDL_GetTicks() / 120U) % 2U) == 0U;
        if (game->round_winner_index == 0) {
            p1_arms = dance_up ? GORILLA_LEFT_UP : GORILLA_RIGHT_UP;
        } else {
            p2_arms = dance_up ? GORILLA_LEFT_UP : GORILLA_RIGHT_UP;
        }
    }

    draw_gorilla(renderer, (int)game->players[0].x, (int)game->players[0].y, p1_arms);
    draw_gorilla(renderer, (int)game->players[1].x, (int)game->players[1].y, p2_arms);

    if (game->projectile.active) {
        draw_banana(
            renderer,
            (int)game->projectile.x,
            (int)game->projectile.y,
            game->projectile.spin_frame,
            game->screen_w
        );
    }

    if (game->phase == GAME_PHASE_ROUND_END && game->hit_player_index >= 0) {
        draw_filled_circle(
            renderer,
            (int)game->explosion_x,
            (int)game->explosion_y,
            (int)game->explosion_radius,
            red
        );
        draw_filled_circle(
            renderer,
            (int)game->explosion_x,
            (int)game->explosion_y,
            (int)(game->explosion_radius * 0.6f),
            yellow
        );
    }

    TTF_SizeText(gfx->font, game->player_names[1], &p2_name_w, &p2_name_h);
    draw_text(renderer, gfx->font, game->player_names[0], 20, 15, white);
    draw_text(renderer, gfx->font, game->player_names[1], game->screen_w - p2_name_w - 20, 15, white);
    {
        char score_line[64];
        snprintf(score_line, sizeof(score_line), "%d >Score< %d", game->scores[0], game->scores[1]);
        draw_text_center(renderer, gfx->font, score_line, game->screen_h - 40, white, game->screen_w);
    }

    if (game->phase == GAME_PHASE_ROUND_END && game->round_winner_index >= 0) {
        char msg[96];
        snprintf(msg, sizeof(msg), "%s scores!", game->player_names[game->round_winner_index]);
        draw_text(renderer, gfx->font, msg, 20, 75, yellow);
    }

    if (game->phase == GAME_PHASE_AIMING) {
        int prompt_y = 45;
        char angle_line[64];
        char velocity_line[64];
        int angle_w = 0;
        int angle_h = 0;
        int velocity_w = 0;
        int velocity_h = 0;
        int angle_x = 20;
        int velocity_x = 20;

        if (game->mouse_aim_enabled) {
            snprintf(angle_line, sizeof(angle_line), "Angle: %.0f", game->players[game->current_player].angle_deg);
        } else {
            snprintf(
                angle_line,
                sizeof(angle_line),
                "Angle: %s%s",
                game->aim_angle_input,
                (game->aim_field == 0) ? "_" : ""
            );
        }
        if (game->current_player == 1) {
            TTF_SizeText(gfx->font, angle_line, &angle_w, &angle_h);
            angle_x = game->screen_w - angle_w - 20;
        }
        draw_text(
            renderer,
            gfx->font,
            angle_line,
            angle_x,
            prompt_y,
            (game->mouse_aim_enabled || game->aim_field == 0) ? yellow : white
        );

        if (game->mouse_aim_enabled || game->aim_field == 1 || game->aim_velocity_input[0] != '\0') {
            if (game->mouse_aim_enabled) {
                snprintf(velocity_line, sizeof(velocity_line), "Velocity: %.0f", game->players[game->current_player].power);
            } else {
                snprintf(
                    velocity_line,
                    sizeof(velocity_line),
                    "Velocity: %s%s",
                    game->aim_velocity_input,
                    (game->aim_field == 1) ? "_" : ""
                );
            }
            if (game->current_player == 1) {
                TTF_SizeText(gfx->font, velocity_line, &velocity_w, &velocity_h);
                velocity_x = game->screen_w - velocity_w - 20;
            }
            draw_text(
                renderer,
                gfx->font,
                velocity_line,
                velocity_x,
                prompt_y + 30,
                (game->mouse_aim_enabled || game->aim_field == 1) ? yellow : white
            );
        }

        if (game->mouse_aim_enabled) {
            render_mouse_aim_preview(renderer, game, &game->players[game->current_player], game->current_player);
        }
    }
}

void graphics_render(SDL_Renderer *renderer, const GraphicsContext *gfx, const GameState *game) {
    if (game->phase == GAME_PHASE_AIMING || game->phase == GAME_PHASE_PROJECTILE ||
        game->phase == GAME_PHASE_ROUND_END) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
    SDL_RenderClear(renderer);

    if (game->phase == GAME_PHASE_SPLASH) {
        render_splash(renderer, gfx, game);
    } else if (game->phase == GAME_PHASE_NAME_ENTRY) {
        render_name_entry(renderer, gfx, game);
    } else if (game->phase == GAME_PHASE_GAME_OVER) {
        render_game_over(renderer, gfx, game);
    } else {
        render_gameplay(renderer, gfx, game);
    }

    SDL_RenderPresent(renderer);
}
