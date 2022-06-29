
#pragma once

#include "types.h"
#include "main.h"
#include "resources.h"
#include "pax_shaders.h"

// Gets a random variant not equal to the given existing.
int random_variant   (int not_this);
// Draws the pole in the right place.
void draw_pole       (bard_t *bard, pole_t *pole);
// Draws the bard.
void draw_bard       (bard_t *bard);
// Draws the background.
void draw_background ();

// Apply physics to all particles.
void render_particles(bard_t *bard);
// Draws all particles.
void draw_particles  (bard_t *bard);
// Delete all particles.
void particle_clear  ();
// Spawns a number of particles, spread around the original position.
void particle_spread (particle_t type, size_t number, float spread_x, float spread_y, spread_t spreading);
// Adds one particle at the original position.
void particle_add    (particle_t type);
