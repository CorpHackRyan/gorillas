#include "game.h"

#include "physics.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void reset_aim_entry(GameState *game);

static int random_range_inclusive(int min_value, int max_value) {
    return min_value + (rand() % (max_value - min_value + 1));
}

static int choose_building_index_for_x(const GameState *game, int target_x) {
    int closest_index = 0;
    int closest_distance = game->screen_w;

    for (int i = 0; i < game->building_count; i++) {
        int center_x = game->buildings[i].x + (game->buildings[i].width / 2);
        int distance = center_x - target_x;
        if (distance < 0) {
            distance = -distance;
        }
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_index = i;
        }
    }

    return closest_index;
}

static void generate_city(GameState *game) {
    int x = 0;
    int skyline_min_y = game->screen_h / 3;
    int skyline_max_y = (game->screen_h * 3) / 4;

    game->building_count = 0;
    while (x < game->screen_w && game->building_count < MAX_BUILDINGS) {
        int width = random_range_inclusive(45, 95);
        int top_y = random_range_inclusive(skyline_min_y, skyline_max_y);
        BuildingState *building = &game->buildings[game->building_count];

        if (x + width > game->screen_w) {
            width = game->screen_w - x;
        }

        building->x = x;
        building->width = width;
        building->top_y = top_y;
        building->r = (unsigned char)random_range_inclusive(45, 100);
        building->g = (unsigned char)random_range_inclusive(45, 100);
        building->b = (unsigned char)random_range_inclusive(70, 140);
        game->building_count++;

        x += width;
    }
}

static void setup_round(GameState *game) {
    generate_city(game);

    game->players[0].building_index = choose_building_index_for_x(game, game->screen_w / 5);
    game->players[0].x = (float)(game->buildings[game->players[0].building_index].x +
                                  game->buildings[game->players[0].building_index].width / 2);
    game->players[0].y = (float)game->buildings[game->players[0].building_index].top_y;
    game->players[0].angle_deg = 45.0f;
    game->players[0].power = 120.0f;

    game->players[1].building_index = choose_building_index_for_x(game, (game->screen_w * 4) / 5);
    game->players[1].x = (float)(game->buildings[game->players[1].building_index].x +
                                  game->buildings[game->players[1].building_index].width / 2);
    game->players[1].y = (float)game->buildings[game->players[1].building_index].top_y;
    game->players[1].angle_deg = 45.0f;
    game->players[1].power = 120.0f;

    game->projectile.active = 0;
    game->projectile.x = 0.0f;
    game->projectile.y = 0.0f;
    game->projectile.vx = 0.0f;
    game->projectile.vy = 0.0f;
    game->projectile.owner_index = -1;
    game->projectile.ignore_owner_collision = 0;

    for (int i = 0; i < MAX_CRATERS; i++) {
        game->craters[i].active = 0;
        game->craters[i].x = 0.0f;
        game->craters[i].y = 0.0f;
        game->craters[i].radius = 0.0f;
    }
    game->crater_cursor = 0;
    game->hit_player_index = -1;
    game->round_winner_index = -1;
    game->round_end_timer = 0.0f;
    game->explosion_radius = 0.0f;
    reset_aim_entry(game);
}

static float clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static void set_default_player_name(GameState *game, int player_index) {
    if (player_index == 0) {
        strcpy(game->player_names[player_index], "Player 1");
    } else {
        strcpy(game->player_names[player_index], "Player 2");
    }
}

static void append_player_name_text(GameState *game, int player_index, const char *text) {
    char *name = game->player_names[player_index];
    size_t len = strlen(name);

    for (size_t i = 0; text[i] != '\0' && len < 10; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c >= 32 && c <= 126) {
            name[len++] = (char)c;
        }
    }
    name[len] = '\0';
}

static void backspace_player_name(GameState *game, int player_index) {
    char *name = game->player_names[player_index];
    size_t len = strlen(name);
    if (len > 0) {
        name[len - 1] = '\0';
    }
}

static void finalize_name_if_empty(GameState *game, int player_index) {
    if (game->player_names[player_index][0] == '\0') {
        set_default_player_name(game, player_index);
    }
}

static void backspace_text(char *text) {
    size_t len = strlen(text);
    if (len > 0) {
        text[len - 1] = '\0';
    }
}

static void reset_aim_entry(GameState *game) {
    game->aim_field = 0;
    game->aim_angle_input[0] = '\0';
    game->aim_velocity_input[0] = '\0';
}

static char *active_aim_buffer(GameState *game) {
    return (game->aim_field == 0) ? game->aim_angle_input : game->aim_velocity_input;
}

static void append_numeric_text(char *buffer, size_t capacity, const char *text) {
    size_t len = strlen(buffer);
    int has_dot = strchr(buffer, '.') != NULL;

    for (size_t i = 0; text[i] != '\0' && len + 1 < capacity; i++) {
        char c = text[i];
        if (c >= '0' && c <= '9') {
            buffer[len++] = c;
        } else if (c == '.' && !has_dot) {
            buffer[len++] = c;
            has_dot = 1;
        }
    }
    buffer[len] = '\0';
}

static int parse_valid_number(const char *text, float *out_value) {
    char *end = NULL;
    float value = strtof(text, &end);
    if (end == text || *end != '\0') {
        return 0;
    }
    if (value <= 0.0f || value > 360.0f) {
        return 0;
    }
    *out_value = value;
    return 1;
}

static int parse_positive_number(const char *text, float *out_value) {
    char *end = NULL;
    float value = strtof(text, &end);
    if (end == text || *end != '\0') {
        return 0;
    }
    if (value <= 0.0f || value > 99.9f) {
        return 0;
    }
    *out_value = value;
    return 1;
}

void game_init(GameState *game, int screen_w, int screen_h) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    game->running = 1;
    game->screen_w = screen_w;
    game->screen_h = screen_h;
    game->street_y = screen_h - 24;
    game->phase = GAME_PHASE_SPLASH;
    game->current_player = 0;
    game->player_names[0][0] = '\0';
    game->player_names[1][0] = '\0';
    game->active_name_index = 0;
    game->gravity_mps2 = 9.8f;
    game->gravity_input[0] = '\0';
    reset_aim_entry(game);
    game->scores[0] = 0;
    game->scores[1] = 0;
    game->wind.accel_x = 5.0f;
    setup_round(game);
}

void game_update(GameState *game, const InputState *input, float dt_seconds) {
    if (input->quit) {
        game->running = 0;
        return;
    }

    if (game->phase == GAME_PHASE_SPLASH) {
        if (input->any_key) {
            game->phase = GAME_PHASE_NAME_ENTRY;
        }
        return;
    }

    if (game->phase == GAME_PHASE_NAME_ENTRY) {
        if (input->backspace) {
            if (game->active_name_index <= 1) {
                backspace_player_name(game, game->active_name_index);
            } else {
                backspace_text(game->gravity_input);
            }
        }

        if (input->text_len > 0) {
            if (game->active_name_index <= 1) {
                append_player_name_text(game, game->active_name_index, input->text);
            } else {
                append_numeric_text(game->gravity_input, sizeof(game->gravity_input), input->text);
            }
        }

        if (input->submit || input->next_field) {
            if (game->active_name_index <= 1) {
                finalize_name_if_empty(game, game->active_name_index);
                game->active_name_index++;
            } else {
                float g = 0.0f;
                if (game->gravity_input[0] == '\0') {
                    game->gravity_mps2 = 9.8f;
                    game->phase = GAME_PHASE_AIMING;
                    reset_aim_entry(game);
                } else if (parse_positive_number(game->gravity_input, &g)) {
                    game->gravity_mps2 = g;
                    game->phase = GAME_PHASE_AIMING;
                    reset_aim_entry(game);
                } else {
                    game->gravity_input[0] = '\0';
                }
            }
        }
        return;
    }

    if (game->phase == GAME_PHASE_AIMING) {
        PlayerState *player = &game->players[game->current_player];
        char *buffer = active_aim_buffer(game);

        if (input->backspace) {
            size_t len = strlen(buffer);
            if (len > 0) {
                buffer[len - 1] = '\0';
            }
        }

        if (input->text_len > 0) {
            append_numeric_text(buffer, 16, input->text);
        }

        if (input->submit) {
            float value = 0.0f;
            if (parse_valid_number(buffer, &value)) {
                if (game->aim_field == 0) {
                    player->angle_deg = clampf(value, 5.0f, 85.0f);
                    game->aim_field = 1;
                } else {
                    player->power = clampf(value, 30.0f, 220.0f);
                    physics_launch_projectile(game, game->current_player);
                    game->phase = GAME_PHASE_PROJECTILE;
                    reset_aim_entry(game);
                }
            } else {
                buffer[0] = '\0';
            }
        }
        return;
    }

    if (game->phase == GAME_PHASE_PROJECTILE) {
        physics_step_projectile(game, dt_seconds);
        return;
    }

    if (game->phase == GAME_PHASE_ROUND_END) {
        game->round_end_timer += dt_seconds;
        game->explosion_radius += 140.0f * dt_seconds;
        if (game->round_end_timer >= 1.5f) {
            game->phase = GAME_PHASE_AIMING;
            game->current_player = game->round_winner_index;
            setup_round(game);
        }
    }
}
