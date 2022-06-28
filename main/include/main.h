
#pragma once

#include "hardware.h"
#include "pax_gfx.h"
#include "pax_codecs.h"
#include "ili9341.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi_connect.h"
#include "wifi_connection.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"

typedef struct bard bard_t;
typedef struct pole pole_t;

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

void disp_flush();
void exit_to_launcher();

uint64_t get_hiscore();
void set_hiscore(uint64_t newscore);
const char *text_hiscore();

void draw_title(pax_col_t col, const char *title, const char *subtitle);
void draw_bard(bard_t *bard);
void render_pole(bard_t *bard, pole_t *pole);
void draw_background();

void mainmenu();
void ingame();
