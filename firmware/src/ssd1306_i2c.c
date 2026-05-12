#include "ssd1306.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "hardware/i2c.h"
#include "ssd1306_font.h"

// ================= I2C LOW LEVEL =================
static void send_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, buf, 2, false);
}

static void send_cmd_list(const uint8_t *buf, int n) {
    for (int i = 0; i < n; i++) send_cmd(buf[i]);
}

// ================= INIT =================
void ssd1306_init(void) {

    uint8_t cmds[] = {
        0xAE,
        0x20, 0x00,
        0x40,

        0xA1, // 🔥 FLIP HORIZONTAL FIX
        0xC8, // 🔥 FLIP VERTICAL FIX (IMPORTANTE)

        0xA8, SSD1306_HEIGHT - 1,
        0xD3, 0x00,

        0xDA, (SSD1306_WIDTH == 128 && SSD1306_HEIGHT == 32) ? 0x02 : 0x12,

        0xD5, 0x80,
        0xD9, 0xF1,
        0xDB, 0x30,

        0x81, 0xFF,
        0xA4,
        0xA6,

        0x8D, 0x14,
        0xAF
    };

    send_cmd_list(cmds, sizeof(cmds));
}

// ================= AREA =================
void ssd1306_calc_area(ssd1306_render_area_t *area) {
    area->buflen =
        (area->end_col - area->start_col + 1) *
        (area->end_page - area->start_page + 1);
}

// ================= SEND BUFFER =================
void ssd1306_send_buffer(uint8_t *buf, ssd1306_render_area_t *area) {

    uint8_t cmd[] = {
        0x21,
        area->start_col,
        area->end_col,
        0x22,
        area->start_page,
        area->end_page
    };

    send_cmd_list(cmd, sizeof(cmd));

    uint8_t *tmp = malloc(area->buflen + 1);
    tmp[0] = 0x40;

    memcpy(tmp + 1, buf, area->buflen);

    i2c_write_blocking(i2c_default, SSD1306_I2C_ADDR, tmp, area->buflen + 1, false);

    free(tmp);
}

// ================= PIXEL =================
void ssd1306_set_pixel(uint8_t *buf, int x, int y, bool on) {

    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
        return;

    int idx = (y / 8) * SSD1306_WIDTH + x;
    uint8_t byte = buf[idx];

    if (on)
        byte |= (1 << (y % 8));
    else
        byte &= ~(1 << (y % 8));

    buf[idx] = byte;
}

// ================= CLEAR =================
void ssd1306_clear(uint8_t *buf) {
    memset(buf, 0, SSD1306_BUF_LEN);
}

// ================= FONT INDEX =================
static int font_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
    if (c >= '0' && c <= '9') return c - '0' + 27;
    return 0;
}

// ================= TEXT (FIX DEFINITIVO) =================
void ssd1306_draw_text(uint8_t *buf, int x, int y, const char *text) {

    while (*text) {

        char c = toupper(*text);
        int idx = font_index(c);

        for (int col = 0; col < 8; col++) {

            uint8_t line = font[idx * 8 + col];

            for (int row = 0; row < 8; row++) {

                // 🔥 FIX DEFINITIVO (SEM INVERSÃO)
                bool pixel_on = line & (1 << row);

                ssd1306_set_pixel(buf, x + col, y + row, pixel_on);
            }
        }

        x += 8;
        text++;
    }
}

// ================= LINE =================
void ssd1306_draw_line(uint8_t *buf, int x0, int y0, int x1, int y1) {

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (1) {

        // 🔥 CLAMP GARANTIDO
        if (x0 >= 0 && x0 < SSD1306_WIDTH &&
            y0 >= 0 && y0 < SSD1306_HEIGHT) {
            ssd1306_set_pixel(buf, x0, y0, true);
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// ================= RENDER =================
void ssd1306_render_full(uint8_t *buf, ssd1306_render_area_t *area) {
    ssd1306_send_buffer(buf, area);
}