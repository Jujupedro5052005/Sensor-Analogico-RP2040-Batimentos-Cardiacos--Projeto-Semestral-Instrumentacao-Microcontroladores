#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

// ================= CONFIG =================
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  32

#define SSD1306_I2C_ADDR 0x3C

#define SSD1306_PAGE_HEIGHT 8
#define SSD1306_NUM_PAGES (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN (SSD1306_NUM_PAGES * SSD1306_WIDTH)

// ================= AREA =================
typedef struct {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;
    int buflen;
} ssd1306_render_area_t;

// ================= API =================
void ssd1306_init(void);

void ssd1306_calc_area(ssd1306_render_area_t *area);

void ssd1306_send_buffer(uint8_t *buf, ssd1306_render_area_t *area);

void ssd1306_clear(uint8_t *buf);

void ssd1306_set_pixel(uint8_t *buf, int x, int y, bool on);

void ssd1306_draw_text(uint8_t *buf, int x, int y, const char *text);

void ssd1306_draw_line(uint8_t *buf, int x0, int y0, int x1, int y1);

void ssd1306_render_full(uint8_t *buf, ssd1306_render_area_t *area);

#endif