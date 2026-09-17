#define F_CPU 16000000UL
#include "ntuaboard.h"

/* Rotating 3D wireframe cube -- real vertex rotation + perspective
 * projection, rasterized into a tiny 20x16 pixel virtual framebuffer.
 *
 * The LCD can't address arbitrary pixels -- only 8 "CGRAM" custom
 * character slots exist, each a 5-wide x 8-tall pixel glyph, and every
 * on-screen cell showing character code 0-7 displays whatever's
 * currently in that CGRAM slot. Tiling all 8 slots in a 4-wide x 2-tall
 * block of character cells gives one 20x16 pixel region we fully
 * control: 4*5=20 columns, 2*8=16 rows. Each frame: rotate the cube's 8
 * vertices, project them into that 20x16 space, draw the 12 edges into
 * a pixel buffer, then re-upload all 8 CGRAM glyphs from it. The 8
 * character codes placed on screen never change -- only what's in
 * CGRAM does, which the display shows immediately.
 */

#define FB_W 20
#define FB_H 16
#define CUBE_S 6  /* cube half-size */
#define TILT_COS 55 /* cos(30 degrees) * 64 -- fixed camera tilt, see rotate_project() */
#define TILT_SIN 32 /* sin(30 degrees) * 64 */

/* cos()/sin() * 64, for angle = index * 15 degrees (24 steps/revolution) */
static const int8_t COS_T[24] = {64, 62, 55, 45, 32, 17, 0, -17, -32, -45, -55, -62,
                                  -64, -62, -55, -45, -32, -17, 0, 17, 32, 45, 55, 62};
static const int8_t SIN_T[24] = {0, 17, 32, 45, 55, 62, 64, 62, 55, 45, 32, 17,
                                  0, -17, -32, -45, -55, -62, -64, -62, -55, -45, -32, -17};

static void set_pixel(uint8_t fb[][FB_W], int8_t x, int8_t y) {
    if (x < 0 || x >= FB_W || y < 0 || y >= FB_H)
        return; /* off the edge of our little "screen" -- just drop it */
    fb[y][x] = 1;
}

static void draw_line(uint8_t fb[][FB_W], int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
    int8_t dx = (int8_t)(x1 - x0);
    int8_t dy = (int8_t)(y1 - y0);
    uint8_t adx = (uint8_t)(dx < 0 ? -dx : dx);
    uint8_t ady = (uint8_t)(dy < 0 ? -dy : dy);
    uint8_t steps = adx > ady ? adx : ady;

    if (steps == 0) {
        set_pixel(fb, x0, y0);
        return;
    }
    for (uint8_t i = 0; i <= steps; i++) {
        int8_t x = (int8_t)(x0 + (int16_t) dx * i / steps);
        int8_t y = (int8_t)(y0 + (int16_t) dy * i / steps);
        set_pixel(fb, x, y);
    }
}

/* Rotates cube vertex `vx_bits` (its 3 low bits pick +-CUBE_S per axis)
 * around the Y axis by angle_idx * 15 degrees (the animated spin), then
 * tilts the whole cube a fixed 30 degrees around the X axis (a static
 * "camera angle", like looking down at it slightly, so more than one
 * face is ever visible) and projects it orthographically -- straight
 * down onto the XY plane, depth (Z) just discarded, no perspective
 * divide. This is the classic axonometric "wireframe cube" look. */
static void rotate_project(uint8_t angle_idx, uint8_t vx_bits, int8_t *out_x, int8_t *out_y) {
    int16_t x = (vx_bits & 1) ? CUBE_S : -CUBE_S;
    int16_t y = (vx_bits & 2) ? CUBE_S : -CUBE_S;
    int16_t z = (vx_bits & 4) ? CUBE_S : -CUBE_S;

    int8_t c = COS_T[angle_idx];
    int8_t s = SIN_T[angle_idx];

    /* spin around Y */
    int16_t rx = (int16_t)((x * c - z * s) / 64);
    int16_t ry = y;
    int16_t rz = (int16_t)((x * s + z * c) / 64);

    /* fixed tilt around X */
    int16_t ty = (int16_t)((ry * TILT_COS + rz * TILT_SIN) / 64);

    *out_x = (int8_t)(FB_W / 2 + rx);
    *out_y = (int8_t)(FB_H / 2 - ty);
}

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();

    /* Place the 8 custom-character cells once: a 4x2 block, centered
     * ((16-4)/2 = 6 columns of padding). Their CGRAM content is what
     * actually changes every frame -- these character codes never do. */
    lcd_command(0x86); /* row 1, column 6 */
    for (uint8_t i = 0; i < 4; i++)
        lcd_data(i);
    lcd_command(0xC6); /* row 2, column 6 */
    for (uint8_t i = 4; i < 8; i++)
        lcd_data(i);

    static uint8_t framebuf[FB_H][FB_W];
    uint8_t angle = 0;

    while (1) {
        for (uint8_t r = 0; r < FB_H; r++)
            for (uint8_t c = 0; c < FB_W; c++)
                framebuf[r][c] = 0;

        int8_t px[8], py[8];
        for (uint8_t v = 0; v < 8; v++)
            rotate_project(angle, v, &px[v], &py[v]);

        /* 12 cube edges: connect each vertex to the 3 neighbors that
         * differ by one bit, only when that neighbor's index is higher
         * (so each edge is drawn once, not twice). */
        for (uint8_t v = 0; v < 8; v++) {
            if ((v ^ 1) > v)
                draw_line(framebuf, px[v], py[v], px[v ^ 1], py[v ^ 1]);
            if ((v ^ 2) > v)
                draw_line(framebuf, px[v], py[v], px[v ^ 2], py[v ^ 2]);
            if ((v ^ 4) > v)
                draw_line(framebuf, px[v], py[v], px[v ^ 4], py[v ^ 4]);
        }

        /* Pack the 20x16 framebuffer into the 8 CGRAM glyphs (4 wide x
         * 2 tall cells of 5x8 pixels each) and upload them. */
        for (uint8_t cell = 0; cell < 8; cell++) {
            uint8_t col0 = (uint8_t)((cell % 4) * 5);
            uint8_t row0 = (uint8_t)((cell / 4) * 8);

            lcd_command((uint8_t)(0x40 | (cell << 3))); /* set CGRAM address */
            for (uint8_t r = 0; r < 8; r++) {
                uint8_t bits = 0;
                for (uint8_t c = 0; c < 5; c++) {
                    if (framebuf[row0 + r][col0 + c])
                        bits |= (uint8_t)(0x10 >> c); /* bit4=leftmost col, bit0=rightmost */
                }
                lcd_data(bits);
            }
        }

        angle = (uint8_t)((angle + 1) % 24);
        _delay_ms(400);
    }
}
