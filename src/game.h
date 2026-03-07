#ifndef GAME_H
#define GAME_H

#include "input.h"

typedef enum GamePhase {
    GAME_PHASE_SPLASH = 0,
    GAME_PHASE_NAME_ENTRY,
    GAME_PHASE_AIMING,
    GAME_PHASE_PROJECTILE,
    GAME_PHASE_ROUND_END,
    GAME_PHASE_GAME_OVER
} GamePhase;

typedef struct PlayerState {
    float x;
    float y;
    float angle_deg;
    float power;
    int building_index;
} PlayerState;

typedef struct ProjectileState {
    float x;
    float y;
    float vx;
    float vy;
    float flight_t;
    int active;
    int owner_index;
    int ignore_owner_collision;
    int spin_frame;
} ProjectileState;

typedef struct WindState {
    float accel_x;
} WindState;

enum { MAX_BUILDINGS = 32 };

typedef struct BuildingState {
    int x;
    int width;
    int top_y;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} BuildingState;

enum { MAX_CRATERS = 64 };

typedef struct CraterState {
    float x;
    float y;
    float radius;
    int active;
} CraterState;

typedef struct GameState {
    int running;
    int screen_w;
    int screen_h;
    int street_y;
    GamePhase phase;
    int current_player;
    char player_names[2][11];
    int active_name_index;
    float gravity_mps2;
    char gravity_input[16];
    int points_to_win;
    char points_input[8];
    int mouse_aim_enabled;
    char mouse_aim_input[8];
    int scores[2];
    int aim_field;
    char aim_angle_input[16];
    char aim_velocity_input[16];
    PlayerState players[2];
    ProjectileState projectile;
    WindState wind;
    BuildingState buildings[MAX_BUILDINGS];
    int building_count;
    CraterState craters[MAX_CRATERS];
    int crater_cursor;
    int hit_player_index;
    int round_winner_index;
    int match_winner_index;
    int sun_shocked;
    float round_end_timer;
    float explosion_x;
    float explosion_y;
    float explosion_radius;
} GameState;

void game_init(GameState *game, int screen_w, int screen_h);
void game_update(GameState *game, const InputState *input, float dt_seconds);

#endif
