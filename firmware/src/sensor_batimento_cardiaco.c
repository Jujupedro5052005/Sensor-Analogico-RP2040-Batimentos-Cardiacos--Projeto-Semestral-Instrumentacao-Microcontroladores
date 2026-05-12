#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define LED 25

// ================= BATIMENTO SIMPLES =================
static const int heartbeat[] = {
    0, 2, 5, 10, 20, 10, 5, 2, 0, 0,
    3, 6, 12, 25, 12, 6, 3, 0
};

int main() {

    stdio_init_all();

    // I2C
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    // LED
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    // OLED
    ssd1306_init();

    uint8_t buf[SSD1306_BUF_LEN];

    ssd1306_render_area_t area = {
        .start_col = 0,
        .end_col = 127,
        .start_page = 0,
        .end_page = SSD1306_NUM_PAGES - 1
    };

    ssd1306_calc_area(&area);

    // =========================================================
    // 1. TESTE TEXTO SIMPLES
    // =========================================================
    ssd1306_clear(buf);
    ssd1306_draw_text(buf, 0, 0, "SSD1306 READY");
    ssd1306_draw_text(buf, 0, 16, "PICO TEST OK");
    ssd1306_render_full(buf, &area);

    sleep_ms(2000);

    // =========================================================
    // 2. GRID VISUAL (DEBUG GEOMETRICO)
    // =========================================================
    ssd1306_clear(buf);

    // linhas verticais
    for (int x = 0; x < SSD1306_WIDTH; x += 8) {
        ssd1306_draw_line(buf, x, 0, x, SSD1306_HEIGHT - 1);
    }

    // linhas horizontais
    for (int y = 0; y < SSD1306_HEIGHT; y += 8) {
        ssd1306_draw_line(buf, 0, y, SSD1306_WIDTH - 1, y);
    }

    ssd1306_render_full(buf, &area);

    sleep_ms(2000);

    // =========================================================
    // 3. DIAGONAL SWEEP
    // =========================================================
    ssd1306_clear(buf);

    for (int i = 0; i < 128; i += 4) {
        ssd1306_draw_line(buf, 0, 31, i, 0);
        ssd1306_render_full(buf, &area);
    }

    sleep_ms(1000);

    // =========================================================
    // 4. "HEARTBEAT MONITOR"
    // =========================================================
    ssd1306_clear(buf);

    int x = 0;

    while (x < 120) {

        ssd1306_clear(buf);

        ssd1306_draw_text(buf, 0, 0, "HEART RATE");

        for (int i = 0; i < 16; i++) {

            int y = 31 - heartbeat[i];

            ssd1306_set_pixel(buf, x + i, y, true);

            if (i > 0) {
                int prev_y = 31 - heartbeat[i - 1];
                ssd1306_draw_line(buf, x + i - 1, prev_y, x + i, y);
            }
        }

        ssd1306_render_full(buf, &area);

        x += 2;
        sleep_ms(80);
    }

    sleep_ms(1000);

    // =========================================================
    // 5. SCANNER STYLE (tipo radar)
    // =========================================================
    ssd1306_clear(buf);

    for (int i = 0; i < 128; i++) {

        ssd1306_clear(buf);

        ssd1306_draw_text(buf, 0, 0, "SCANNING...");

        ssd1306_draw_line(buf, i, 0, i, 31);

        ssd1306_render_full(buf, &area);

        sleep_ms(10);
    }

    // =========================================================
    // 6. FILL TEST (FULL SCREEN BLINK)
    // =========================================================
    for (int k = 0; k < 3; k++) {

        for (int i = 0; i < SSD1306_BUF_LEN; i++) {
            buf[i] = 0xFF;
        }

        ssd1306_render_full(buf, &area);
        sleep_ms(300);

        ssd1306_clear(buf);
        ssd1306_render_full(buf, &area);
        sleep_ms(300);
    }

    // =========================================================
    // LOOP FINAL
    // =========================================================
    while (1) {

        gpio_put(LED, 1);
        sleep_ms(200);
        gpio_put(LED, 0);
        sleep_ms(200);
    }
}