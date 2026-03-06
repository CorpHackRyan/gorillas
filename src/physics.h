#ifndef PHYSICS_H
#define PHYSICS_H

#include "game.h"

void physics_launch_projectile(GameState *game, int player_index);
void physics_step_projectile(GameState *game, float dt_seconds);

#endif
