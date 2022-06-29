
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

#include "types.h"
#include "artwork.h"

// Flush the buffer to screen.
void disp_flush();
// Exit to the launcher.
void exit_to_launcher();

// Gets or reads from NVS, the high score.
uint64_t get_hiscore();
// Update in NVS, the new high score.
void set_hiscore(uint64_t newscore);
// Get text in the format "High score: %d".
const char *text_hiscore();
// Draws a title and optional subtitle in the middle of the screen.
void draw_title(pax_col_t col, const char *title, const char *subtitle);

// Renders pole physics.
void render_pole(bard_t *bard, pole_t *pole);
// Main menu loop.
void mainmenu();
// Level loop.
void ingame();
