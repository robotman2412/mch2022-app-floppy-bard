
#include "artwork.h"

static const char *TAG = "artwork";

particle_t *particles;

static const variant_t variants[] = {
    { // Green poles.
        .color    = 0xff00b000,
        .filename = NULL,
    }, { // Orange poles.
        .color    = 0xfff09000,
        .filename = NULL,
    }, { // Blue poles.
        .color    = 0xff0000f0,
        .filename = NULL,
    }, { // Purple poles.
        .color    = 0xffa000f0,
        .filename = NULL,
    }
};
static const size_t num_variants = sizeof(variants) / sizeof(variant_t);

// Gets a random variant not equal to the given existing.
int random_variant(int not_this) {
    // Max is one less than number of variants if one is skipped.
    uint64_t max = (not_this == -1) ? (num_variants) : (num_variants - 1);
    // Get a number in said range.
    int nombre = (esp_random() * max) >> 32;
    // Skip the excluded number.
    if (not_this != -1 && nombre >= not_this) nombre ++;
    return nombre;
}

// Draws the pole in the right place.
void draw_pole(bard_t *bard, pole_t *pole) {
    // Find relative position.
    float x   = pole->x - bard->level_pos;
    float y   = pole->y;
    float gap = pole->gap;
    
    // Draw.
    pax_col_t col = 0xff00ff00;
    if (pole->variant >= 0 && pole->variant < num_variants) {
        col = variants[pole->variant].color;
    }
    pax_draw_rect(&buf, col, x, 0, POLE_WIDTH, y - gap);
    pax_draw_rect(&buf, col, x, y, POLE_WIDTH, buf.height - y - 30);
    
    // Hitbox visualisation.
    x -= bard->x;
    if (SHOW_HITBOXES(bard)) {
        x += bard->x;
        pax_outline_rect(&buf, -1, x+POLE_LENIENCE, 0, POLE_WIDTH-POLE_LENIENCE*2, y - gap - POLE_LENIENCE);
        pax_outline_rect(&buf, -1, x+POLE_LENIENCE, y+POLE_LENIENCE, POLE_WIDTH-POLE_LENIENCE*2, buf.height - y - 30 - POLE_LENIENCE);
    }
}

// Draws the bard.
void draw_bard(bard_t *bard) {
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(bard->x, bard->y));
    pax_apply_2d(&buf, matrix_2d_rotate(bard->angle));
    pax_draw_rect(&buf, 0xffff0000, -15, -15, 30, 30);
    pax_pop_2d(&buf);
    if (SHOW_HITBOXES(bard)) {
        pax_outline_rect(&buf, -1, bard->x-HITBOX_RADIUS, bard->y-HITBOX_RADIUS, HITBOX_RADIUS*2, HITBOX_RADIUS*2);
    }
}

// Draws the background.
void draw_background() {
    pax_background(&buf, 0xff00e0f0);
    pax_simple_rect(&buf, 0xff009000, 0, buf.height-30, buf.width, 30);
}



// Delete a particle and unlink it.
static void particle_delete(particle_t *part) {
    // Unlink it.
    if (part->prev) {
        part->prev->next = part->next;
    } else {
        particles = part->next;
    }
    if (part->next) {
        part->next->prev = part->prev;
    }
    // Free memory.
    free(part);
}

// Apply physics to all particles.
void render_particles(bard_t *bard) {
    for (particle_t *cur = particles; cur; cur = cur->next) {
        // Apply velocity.
        cur->x += cur->vx;
        cur->y += cur->vy;
        // Apply acceleration.
        cur->vx += cur->gx;
        cur->vy += cur->gy;
        // Apply drag.
        cur->vx *= 1 - cur->drag;
        cur->vy *= 1 - cur->drag;
        // Like a fine wine.
        cur->age ++;
    }
}

// Draws all particles.
void draw_particles(bard_t *bard) {
    for (particle_t *cur = particles; cur; cur = cur->next) {
        pax_push_2d(&buf);
        pax_apply_2d(&buf, matrix_2d_translate(cur->x - bard->level_pos, cur->y));
        
        pax_buf_t *rsrc = resource_get(cur->filename);
        if (rsrc) {
            float part = cur->age > cur->lifespan ? 0 : (cur->lifespan - cur->age) / (float) cur->lifespan;
            if (part) {
                pax_col_t tint = (pax_col_t) (0xff000000 * part) | 0x00ffffff;
                // tint = pax_col_tint(cur->color, tint);
                pax_shade_rect(
                    &buf, tint,
                    &PAX_SHADER_TEXTURE(rsrc), NULL, 
                    -rsrc->width/2, -rsrc->height/2,
                    rsrc->width,    rsrc->height
                );
            }
        }
        
        pax_pop_2d(&buf);
    }
}

// Delete all particles.
void particle_clear() {
    while (particles) {
        particle_delete(particles);
    }
}

// Spawns a number of particles, spread around the original position.
void particle_spread(particle_t type, size_t number, float spread_x, float spread_y, spread_t spreading) {
    bool repel = spreading & 1;
    spreading &= ~1;
    
    // TODO: Other spread modes?
    
    // Simple rectangle spread.
    for (size_t i = 0; i < number; i++) {
        particle_t part = type;
        part.x += ((int) esp_random()) / (float) INT32_MAX * spread_x;
        part.y += ((int) esp_random()) / (float) INT32_MAX * spread_y;
        
        if (repel) {
            float speed = 2.0;
            if (spread_x)
                part.vx = (part.x - type.x) / spread_x * speed;
            if (spread_y)
                part.vy = (part.y - type.y) / spread_y * speed;
        }
        particle_add(part);
    }
}

// Adds one particle at the original position.
void particle_add(particle_t part) {
    // Link it to the list.
    part.prev       = NULL;
    part.next       = particles;
    // Allocate memory.
    particle_t *mem = malloc(sizeof(particle_t));
    *mem            = part;
    if (particles) {
        particles->prev = mem;
    }
    particles = mem;
}
