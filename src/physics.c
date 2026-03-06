#include "physics.h"
#include "sound.h"

#include <math.h>

static float clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static int point_in_crater(const GameState *game, float x, float y) {
    for (int i = 0; i < MAX_CRATERS; i++) {
        if (!game->craters[i].active) {
            continue;
        }
        float dx = x - game->craters[i].x;
        float dy = y - game->craters[i].y;
        float rr = game->craters[i].radius * game->craters[i].radius;
        if (dx * dx + dy * dy <= rr) {
            return 1;
        }
    }
    return 0;
}

static int point_hits_solid_building(const GameState *game, float x, float y) {
    int px = (int)x;
    int py = (int)y;

    for (int i = 0; i < game->building_count; i++) {
        const BuildingState *b = &game->buildings[i];
        if (px >= b->x && px <= b->x + b->width && py >= b->top_y && py <= game->street_y) {
            if (!point_in_crater(game, x, y)) {
                return 1;
            }
            return 0;
        }
    }

    return 0;
}

static void add_crater(GameState *game, float x, float y, float radius) {
    CraterState *c = &game->craters[game->crater_cursor];
    c->active = 1;
    c->x = x;
    c->y = y;
    c->radius = radius;
    game->crater_cursor = (game->crater_cursor + 1) % MAX_CRATERS;
}

static int point_hits_gorilla(const GameState *game, int player_index, float x, float y) {
    const float gorilla_half_w = 24.0f;
    const float gorilla_h = 48.0f;
    const float left = game->players[player_index].x - gorilla_half_w;
    const float right = game->players[player_index].x + gorilla_half_w;
    const float top = game->players[player_index].y - gorilla_h;
    const float bottom = game->players[player_index].y;

    return x >= left && x <= right && y >= top && y <= bottom;
}

static void resolve_gorilla_hit(GameState *game, int hit_player_index) {
    game->projectile.active = 0;
    game->hit_player_index = hit_player_index;
    game->round_winner_index = 1 - hit_player_index;
    game->scores[game->round_winner_index] += 1;
    game->phase = GAME_PHASE_ROUND_END;
    game->round_end_timer = 0.0f;
    game->explosion_x = game->players[hit_player_index].x;
    game->explosion_y = game->players[hit_player_index].y - 24.0f;
    game->explosion_radius = 4.0f;
    sound_play_gorilla_hit();
}

static void finish_turn(GameState *game) {
    game->projectile.active = 0;
    game->current_player = 1 - game->current_player;
    game->phase = GAME_PHASE_AIMING;
    game->sun_shocked = 0;
    game->aim_field = 0;
    game->aim_angle_input[0] = '\0';
    game->aim_velocity_input[0] = '\0';

    game->players[0].angle_deg = clampf(game->players[0].angle_deg, 5.0f, 85.0f);
    game->players[1].angle_deg = clampf(game->players[1].angle_deg, 5.0f, 85.0f);
}

void physics_launch_projectile(GameState *game, int player_index) {
    PlayerState *player = &game->players[player_index];
    float angle_rad = (float)(player->angle_deg * (3.14159265358979323846 / 180.0));
    float direction = (player_index == 0) ? 1.0f : -1.0f;

    game->projectile.active = 1;
    game->projectile.x = player->x;
    game->projectile.y = player->y - 24.0f;
    game->projectile.vx = cosf(angle_rad) * player->power * direction;
    game->projectile.vy = -sinf(angle_rad) * player->power;
    game->projectile.owner_index = player_index;
    game->projectile.ignore_owner_collision = 1;
    game->sun_shocked = 0;
    sound_play_throw();
}

void physics_step_projectile(GameState *game, float dt_seconds) {
    const float bas_time_units_per_second = 5.0f;
    const float bas_dt = dt_seconds * bas_time_units_per_second;
    const float y_scale = (float)game->screen_h / 350.0f;
    const float wind_accel = game->wind.accel_x / 5.0f;
    const float gravity = game->gravity_mps2;

    if (!game->projectile.active) {
        return;
    }

    game->projectile.vx += wind_accel * bas_dt;
    game->projectile.vy += gravity * bas_dt;
    game->projectile.x += game->projectile.vx * bas_dt;
    game->projectile.y += game->projectile.vy * bas_dt * y_scale;

    {
        const float sun_x = (float)game->screen_w * 0.5f;
        const float sun_y = 70.0f;
        const float sun_r = 28.0f;
        float dx = game->projectile.x - sun_x;
        float dy = game->projectile.y - sun_y;
        if (dx * dx + dy * dy <= sun_r * sun_r) {
            game->sun_shocked = 1;
        }
    }

    if (game->projectile.ignore_owner_collision &&
        game->projectile.owner_index >= 0 &&
        !point_hits_gorilla(
            game,
            game->projectile.owner_index,
            game->projectile.x,
            game->projectile.y
        )) {
        game->projectile.ignore_owner_collision = 0;
    }

    if (!(game->projectile.ignore_owner_collision && game->projectile.owner_index == 0) &&
        point_hits_gorilla(game, 0, game->projectile.x, game->projectile.y)) {
        resolve_gorilla_hit(game, 0);
        return;
    }

    if (!(game->projectile.ignore_owner_collision && game->projectile.owner_index == 1) &&
        point_hits_gorilla(game, 1, game->projectile.x, game->projectile.y)) {
        resolve_gorilla_hit(game, 1);
        return;
    }

    if (game->projectile.x < 0.0f || game->projectile.x > (float)game->screen_w) {
        finish_turn(game);
        return;
    }

    if (game->projectile.y >= (float)game->street_y) {
        add_crater(game, game->projectile.x, (float)game->street_y, 18.0f);
        sound_play_explosion();
        finish_turn(game);
        return;
    }

    if (point_hits_solid_building(game, game->projectile.x, game->projectile.y)) {
        add_crater(game, game->projectile.x, game->projectile.y, 22.0f);
        sound_play_explosion();
        finish_turn(game);
    }
}
