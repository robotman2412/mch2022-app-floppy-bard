
// This file contains a simple hello world app which you can base you own apps on.

#include "main.h"

static uint64_t priv_hiscore;
static const pax_font_t *font_big;
static const pax_font_t *font_small;
static nvs_handle_t game_nvs;
static pax_buf_t buf;
xQueueHandle buttonQueue;

#define JUMP_HEIGHT  -8
#define GRAVITY       1.7
#define HITBOX_RADIUS 15
#define POLE_LENIENCE 5
#define POLE_WIDTH    50
#define EXIT_TIME     1000
#define SHOW_HITBOXES false

static const char *TAG = "floppy-bard-app";

void disp_flush() {
    ili9341_write(get_ili9341(), buf.buf);
}

void exit_to_launcher() {
    REG_WRITE(RTC_CNTL_STORE0_REG, 0);
    for (int i = 0; i < 10; i++) {
        pax_simple_rect(&buf, 0x3fffffff, 0, 0, buf.width, buf.height);
        disp_flush();
    }
    esp_restart();
}



void app_main() {
    // Init HW.
    bsp_init();
    bsp_rp2040_init();
    buttonQueue = get_rp2040()->queue;
    
    // Init GFX.
    pax_buf_init(&buf, NULL, 320, 240, PAX_BUF_16_565RGB);
    pax_enable_multicore(1);
    font_big   = pax_get_font("permanentmarker");
    font_small = pax_get_font("saira regular");
    
    // Init NVS.
    nvs_flash_init();
    esp_err_t res = nvs_open("robotman-app", NVS_READWRITE, &game_nvs);
    if (res) game_nvs = 0;
    
    // Init (but not connect to) WiFi.
    wifi_init();
    
    mainmenu();
}



uint64_t get_hiscore() {
    static bool     read  = false;
    if (!game_nvs) return 0;
    
    if (!read) {
        esp_err_t res = nvs_get_u64(game_nvs, "fbird_hiscore", &priv_hiscore);
        if (res) priv_hiscore = 0;
    }
    
    return priv_hiscore;
}

void set_hiscore(uint64_t newscore) {
    priv_hiscore = newscore;
    esp_err_t res = nvs_set_u64(game_nvs, "fbird_hiscore", newscore);
    nvs_commit(game_nvs);
}

const char *text_hiscore() {
    static char buffer[32];
    static uint64_t last_written = -1;
    
    if (last_written != get_hiscore()) {
        last_written = get_hiscore();
        snprintf(buffer, 32, "High score: %lld", last_written);
    }
    
    return buffer;
}



void draw_title(pax_col_t col, const char *title, const char *subtitle) {
    pax_center_text(&buf, col, font_big, 35, buf.width/2, buf.height/2-35, title);
    pax_center_text(&buf, col, font_small, 18, buf.width/2, buf.height/2, subtitle);
}

void draw_bard(float x, float y, float angle) {
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(x, y));
    pax_apply_2d(&buf, matrix_2d_rotate(angle));
    pax_draw_rect(&buf, 0xffff0000, -15, -15, 30, 30);
    pax_pop_2d(&buf);
    if (SHOW_HITBOXES) {
        pax_outline_rect(&buf, -1, x-HITBOX_RADIUS, y-HITBOX_RADIUS, HITBOX_RADIUS*2, HITBOX_RADIUS*2);
    }
}

void draw_pole(bard_t *bard, pole_t *pole) {
    // Find relative position.
    float x   = pole->x - bard->level_pos;
    float y   = pole->y;
    float gap = pole->gap;
    
    // Draw.
    pax_col_t col = 0xff00ff00;
    pax_draw_rect(&buf, col, x, 0, POLE_WIDTH, y - gap);
    pax_draw_rect(&buf, col, x, y, POLE_WIDTH, buf.height - y - 30);
    
    // Hitbox visualisation.
    x -= bard->x;
    if (x > HITBOX_RADIUS - POLE_LENIENCE || x < HITBOX_RADIUS + POLE_LENIENCE - POLE_WIDTH) return;
    if (SHOW_HITBOXES) {
        x += bard->x;
        pax_outline_rect(&buf, -1, x+POLE_LENIENCE, 0, POLE_WIDTH-POLE_LENIENCE-HITBOX_RADIUS, y - gap - POLE_LENIENCE);
        pax_outline_rect(&buf, -1, x+POLE_LENIENCE, y+POLE_LENIENCE, POLE_WIDTH-POLE_LENIENCE-HITBOX_RADIUS, buf.height - y - 30 - POLE_LENIENCE);
    }
}

void render_pole(bard_t *bard, pole_t *pole) {
    // Find relative position.
    float x   = pole->x - bard->level_pos - bard->x;
    float y   = pole->y;
    float gap = pole->gap;
    
    // Test whether POLE is off screen to the LEFT.
    if (pole->x - bard->level_pos < -POLE_WIDTH) {
        pole->offscreen = true;
    }
    
    // Test whether the POLE is on the screen to the RIGHT.
    if (pole->x - bard->level_pos < buf.width) {
        pole->onscreen = true;
    }
    
    // Test whether POLE is approximately IN RANGE.
    if (x > HITBOX_RADIUS - POLE_LENIENCE) return;
    if (x < HITBOX_RADIUS + POLE_LENIENCE - POLE_WIDTH) {
        if (!pole->counted) {
            pole->counted = true;
            bard->score ++;
        }
        return;
    }
    bool collision   = false;
    bool hits_top    = false;
    bool hits_bottom = false;
    bool hits_edge   = false;
    
    // Center collisions.
    if (bard->y < y - gap + HITBOX_RADIUS - POLE_LENIENCE) {
        collision   = true;
        hits_top    = true;
    } else if (bard->y > y - HITBOX_RADIUS + POLE_LENIENCE) {
        collision   = true;
        hits_bottom = true;
    }
    // Edge collisions.
    if (x > 0) {
        hits_edge = true;
    }
    
    // Death.
    if (collision) {
        bard->alive = false;
        if (hits_edge) {
            bard->x   = pole->x - bard->level_pos + POLE_LENIENCE - HITBOX_RADIUS;
            bard->vel = -0.1;
            ESP_LOGI(TAG, "Hits edge");
        } else if (hits_top) {
            bard->y   = pole->y - pole->gap - POLE_LENIENCE + HITBOX_RADIUS;
            bard->vel = -JUMP_HEIGHT;
            ESP_LOGI(TAG, "Hits top");
        } else if (hits_bottom) {
            bard->y = pole->y + POLE_LENIENCE - HITBOX_RADIUS;
            bard->vel = JUMP_HEIGHT;
            ESP_LOGI(TAG, "Hits bottom");
        } else {
            ESP_LOGI(TAG, "Hits neither");
        }
    }
}

void draw_background() {
    pax_background(&buf, 0xff00e0f0);
    pax_simple_rect(&buf, 0xff009000, 0, buf.height-30, buf.width, 30);
}



void mainmenu() {
    while (1) {
        uint64_t now = esp_timer_get_time() / 1000;
        draw_background();
        draw_bard(50, 50+sinf(now * M_PI / 2000)*10, sinf(now*M_PI/1000)*M_PI/32);
        draw_title(0xff000000, "Floppy Bard", text_hiscore());
        pax_center_text(
            &buf, 0xff000000, font_small, 18, buf.width/2, buf.height-18,
            "🅷Exit  🅰Start the game"
        );
        disp_flush();
        
        rp2040_input_message_t msg;
        if (xQueueReceive(buttonQueue, &msg, 1) && msg.state) {
            if (msg.input == RP2040_INPUT_BUTTON_HOME) {
                exit_to_launcher();
            } else if (msg.input == RP2040_INPUT_BUTTON_ACCEPT) {
                // Start the game.
                ingame();
            }
        }
    }
}

void ingame() {
    uint64_t exit_time = 0;
    bard_t bard;
    bard.score = 0;
    
    // Set initial position equal to main menu.
    uint64_t now   = esp_timer_get_time() / 1000;
    bard.x         = 50;
    bard.y         = 50+sinf(now * M_PI / 2000)*10;
    bard.angle     = sinf(now*M_PI/1000)*M_PI/32;
    // Start unpaused while jumping.
    bard.vel       = JUMP_HEIGHT;
    bool paused    = false;
    bard.alive     = true;
    // Level position.
    bard.level_pos = 0;
    bard.level_vel = 5;
    bard.pole_dist = 300;
    
    // Initial pole.
    pole_t *poles  = malloc(sizeof(pole_t));
    *poles = (pole_t) {
        .prev      = NULL,
        .next      = NULL,
        .x         = 400,
        .y         = 80,
        .gap       = 70,
        .variant   = 0,
        .counted   = false,
        .offscreen = false,
        .onscreen  = false,
    };
    
    while (1) {
        // Get current time for reference.
        now = esp_timer_get_time() / 1000;
        
        if (!paused) {
            // Apply physics.
            bard.y   += bard.vel;
            bard.vel += GRAVITY;
            
            // Maximum height.
            if (bard.y < -40) {
                bard.y = -40;
            }
            
            // Minimum height.
            if (bard.y > buf.height - 30 - HITBOX_RADIUS) {
                bard.y = buf.height - 30 - HITBOX_RADIUS;
                // Bounce off the floor.
                bard.vel *= -0.5;
                bard.vel += GRAVITY*3;
                if (bard.vel > 0) bard.vel = 0;
                // Game over.
                bard.alive = false;
            }
            
            // Bard angle.
            if (fabsf(bard.vel) > 0.3) {
                float angle_target = M_PI / 6 * bard.vel / JUMP_HEIGHT;
                float angle_error  = angle_target - bard.angle;
                bard.angle = angle_target - 0.7 * angle_error;
            }
            
            // Level physics.
            if (bard.alive) {
                bard.level_pos += bard.level_vel;
                
                // Check whether a pole must be removed.
                if (poles->offscreen) {
                    // Unlink it from the list.
                    void *to_free = poles;
                    poles->next->prev = NULL;
                    poles = poles->next;
                    free(to_free);
                }
                
                for (pole_t *cur = poles; cur; cur = cur->next) {
                    render_pole(&bard, cur);
                    
                    // Check whether a pole must be added.
                    if (!cur->next && cur->onscreen) {
                        // Add the next pole.
                        pole_t *next = malloc(sizeof(pole_t));
                        *next = (pole_t) {
                            .prev      = cur,
                            .next      = NULL,
                            .x         = cur->x + POLE_WIDTH + bard.pole_dist,
                            .gap       = 70,
                            .variant   = 0,
                            .counted   = false,
                            .offscreen = false,
                            .onscreen  = false,
                        };
                        // Randomise it's vertical position.
                        next->y = esp_random() / (float) UINT32_MAX;
                        const float bottom = buf.height - 30 - POLE_LENIENCE * 2;
                        const float top    = POLE_LENIENCE * 2 + next->gap;
                        next->y = top + (bottom - top) * next->y;
                        // Link it to the list.
                        cur->next = next;
                    }
                }
            }
        }
        
        // Draw scene.
        draw_background();
        for (pole_t *cur = poles; cur; cur = cur->next) {
            draw_pole(&bard, cur);
        }
        draw_bard(bard.x, bard.y, bard.angle);
        
        // Text
        if (paused) {
            draw_title(0xff000000, "Paused", NULL);
            pax_center_text(
                &buf, 0xff000000, font_small, 18, buf.width/2, buf.height-18,
                "🅰Jump and unpause  🅱Unpause"
            );
        } else if (bard.alive) {
            pax_center_text(
                &buf, 0xff000000, font_small, 18, buf.width/2, buf.height-18,
                "🅰Jump  🅱Pause"
            );
        }
        // Score.
        char temp[16];
        snprintf(temp, 16, "%lld", bard.score);
        pax_center_text(&buf, 0xff000000, font_big, 35, buf.width/2, 5, temp);
        disp_flush();
        
        // Game over delay.
        if (!bard.alive) {
            if (!exit_time && bard.vel == 0) {
                // Set exit timer.
                exit_time = esp_timer_get_time() / 1000 + EXIT_TIME;
                // Update high score.
                if (bard.score > get_hiscore()) set_hiscore(bard.score);
            } else if (exit_time && now >= exit_time) {
                return;
            }
        }
        
        rp2040_input_message_t msg;
        if (xQueueReceive(buttonQueue, &msg, 1) && msg.state) {
            if (msg.input == RP2040_INPUT_BUTTON_ACCEPT && bard.alive) {
                // Jump.
                bard.vel = JUMP_HEIGHT;
                paused   = false;
            } else if (msg.input == RP2040_INPUT_BUTTON_BACK && bard.alive) {
                // Pause.
                paused   = !paused;
            }
        }
    }
}
