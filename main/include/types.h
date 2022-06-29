
#pragma once

#include "hardware.h"
#include "pax_gfx.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_system.h"

typedef enum {
    SPREAD_RECTANGULAR,
    REPEL_RECTANGULAR,
} spread_t;

typedef struct bard bard_t;
typedef struct pole pole_t;
typedef struct variant variant_t;
typedef struct particle particle_t;

struct bard {
    /* ==== Position ==== */
    // The bard's position on screen.
    float    x, y;
    // The bard's vertical velocity.
    float    vel;
    // The bard's angle.
    float    angle;
    /* ==== Level state ==== */
    // Whether the game is still going.
    bool     alive;
    // The relative position of the level.
    float    level_pos;
    // The speed at which the level scrolls by.
    float    level_vel;
    // The distance between poles.
    float    pole_dist;
    // The gap of new poles.
    float    pole_gap;
    // Whether the game is paused.
    bool     paused;
    /* ==== Miscellaneous ==== */
    // Current score.
    uint64_t score;
    // Next score to increase difficulty after.
    uint64_t next_diff;
    // Current variant to draw the poles as.
    int      pole_variant;
    // The number of poles produced so far.
    int      num_poles;
};

struct pole {
    /* ==== Linked list ==== */
    // The previous pole in the linked list, if any.
    pole_t *prev;
    // The next pole in the linked list, if any.
    pole_t *next;
    /* ==== Position ==== */
    // The pole's position in the level.
    float   x, y;
    // The pole's gap size.
    float   gap;
    /* ==== Miscellaneous ==== */
    // The visual variation of the pole.
    int     variant;
    // Whether the pole has been added to the score yet.
    bool    counted;
    // Whether the pole has exited from the left of the screen.
    bool    offscreen;
    // Whether the pole has entered from the right of the screen.
    bool    onscreen;
};

struct variant {
    // Tint or color of the variant.
    pax_col_t color;
    // Filename of the variant's image, if any.
    const char *filename;
};

struct particle {
    /* ==== Linked list ==== */
    // The previous particle in the linked list, if any.
    particle_t *prev;
    // The next particle in the linked list, if any.
    particle_t *next;
    /* ==== Position ==== */
    // Position of particle (in pixels).
    float       x, y;
    // Velocity of particle (in pixels per frame).
    float       vx, vy;
    // Gravity of particle (in constant acceleration applied).
    float       gx, gy;
    // Ari resistance of particle (in parts of velocity).
    float       drag;
    /* ==== Miscellaneous ==== */
    // The filename of the particle's image.
    const char *filename;
    // The color tint of the particle.
    pax_col_t   color;
    // The time that the particle lives for (in frames).
    int         lifespan;
    // The current age of the particle.
    int         age;
};

// Default dust particle with a given X/Y.
#define PARTICLE_DUST(particle_x, particle_y) (particle_t) {\
        .prev     = NULL,\
        .next     = NULL,\
        .x        = particle_x,\
        .y        = particle_y,\
        .vx       = 0,\
        .vy       = 0,\
        .gx       = 0,\
        .gy       = 0,\
        .drag     = 0.2,\
        .filename = "dust.png",\
        .color    = 0xffffffff,\
        .lifespan = 20,\
        .age      = 0,\
    }

#define JUMP_HEIGHT  -8
#define GRAVITY       2.0
#define HITBOX_RADIUS 15
#define POLE_LENIENCE 3
#define POLE_WIDTH    50
#define EXIT_TIME     2000

#define INITIAL_POLE_GAP  90
#define MIN_POLE_GAP      50
#define INITIAL_POLE_DIST 250
#define MIN_POLE_DIST     100
#define DIFF_INC_EVERY    5
#define DIFF_FACTOR       0.1

#define SHOW_HITBOXES(bard) (debug_state)
#define DO_DEBUG(bard) ((bard)->paused && debug_state)

extern const pax_font_t *font_big;
extern const pax_font_t *font_small;
extern nvs_handle_t game_nvs;
extern pax_buf_t buf;
extern xQueueHandle buttonQueue;
extern bool debug_state;

extern particle_t *particles;
