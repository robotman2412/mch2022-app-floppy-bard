
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
    float    x, y;
    float    vel;
    float    angle;
    /* ==== Level state ==== */
    bool     alive;
    float    level_pos;
    float    level_vel;
    float    pole_dist;
    /* ==== Scoring ==== */
    uint64_t score;
};

struct pole {
    /* ==== Linked list ==== */
    pole_t *prev;
    pole_t *next;
    /* ==== Position ==== */
    float   x, y;
    float   gap;
    /* ==== Miscellaneous ==== */
    int     variant;
    bool    counted;
    bool    offscreen;
};

void disp_flush();
void exit_to_launcher();

uint64_t get_hiscore();
void set_hiscore(uint64_t newscore);
const char *text_hiscore();

void draw_title(pax_col_t col, const char *title, const char *subtitle);
void draw_bard(float x, float y, float angle);
void render_pole(bard_t *bard, pole_t *pole);
void draw_background();

void mainmenu();
void ingame();
