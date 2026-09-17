#define F_CPU 16000000UL
#include "../ntuaboard.h"
#include <string.h>

/* Two lines: reference for addressing both rows of the 2x16 LCD. Static
 * text, nothing dynamic -- row 1, jump to row 2, row 2 -- each line
 * horizontally centered. */

#define LCD_WIDTH 16

static void lcd_write_centered(uint8_t row, const char *str) {
    uint8_t len = (uint8_t) strlen(str);
    if (len > LCD_WIDTH)
        len = LCD_WIDTH; /* clamp -- longer text just left-aligns/truncates */
    uint8_t pad = (uint8_t)((LCD_WIDTH - len) / 2);

    /* row 0 -> DDRAM 0x00 (command 0x80), row 1 -> DDRAM 0x40 (command
     * 0xC0), then pad columns further right for centering. */
    lcd_command((uint8_t)((row == 0 ? 0x80 : 0xC0) + pad));
    for (uint8_t i = 0; i < len; i++)
        lcd_data((uint8_t) str[i]);
}

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();
    lcd_clear_display();

    lcd_write_centered(0, "Line one!");
    lcd_write_centered(1, "And line two.");

    while (1) {
        /* static text -- nothing left to do */
    }
}
