
// This file contains a simple hello world app which you can base you own apps on.

#include "main.h"

static const pax_font_t *font_big;
static const pax_font_t *font_small;
static pax_buf_t buf;
xQueueHandle buttonQueue;

static const char *TAG = "floppy-bard-app";

void disp_flush() {
    ili9341_write(get_ili9341(), buf.buf);
}

void exit_to_launcher() {
    REG_WRITE(RTC_CNTL_STORE0_REG, 0);
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
    
    // Init (but not connect to) WiFi.
    wifi_init();
    
    mainmenu();
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
}

void draw_background() {
    pax_background(&buf, 0xff00e0f0);
    pax_simple_rect(&buf, 0xff009000, 0, buf.height-30, buf.width, 30);
}



void mainmenu() {
    while (1) {
        draw_background();
        draw_bard(50, 50, 0);
        draw_title(0xff000000, "Floppy Bard", "High score: 0");
        pax_center_text(
            &buf, 0xff000000, font_small, 18, buf.width/2, buf.height-18,
            "🅷Exit  🆂Start the game"
        );
        disp_flush();
        
        rp2040_input_message_t msg;
        if (xQueueReceive(buttonQueue, &msg, portMAX_DELAY) && msg.state) {
            if (msg.input == RP2040_INPUT_BUTTON_HOME) {
                exit_to_launcher();
            } else if (msg.input == RP2040_INPUT_BUTTON_START) {
                // Start the game.
            }
        }
    }
}
