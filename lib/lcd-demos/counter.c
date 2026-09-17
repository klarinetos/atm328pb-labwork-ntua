#define F_CPU 16000000UL
#include "../ntuaboard.h"
#include <stdio.h>

/* Two-line counter: a static label on row 1, a live incrementing count
 * on row 2 -- shows lcd_command()'s cursor-positioning (0x80 | DDRAM
 * address) rather than relying on auto-increment/lcd_string() alone. */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();
    lcd_clear_display();
    lcd_string((const unsigned char *) "Uptime x100ms:");

    uint16_t count = 0;
    char buf[6];

    while (1) {
        lcd_command(0xC0); /* row 2, column 0 (DDRAM 0x40) */
        sprintf(buf, "%-5u", count); /* space-padded: clears leftover
                                       * digits when count wraps and
                                       * shrinks back to fewer digits */
        for (uint8_t i = 0; buf[i]; i++) {
            lcd_data((uint8_t) buf[i]);
        }
        count++;
        _delay_ms(100);
    }
}
