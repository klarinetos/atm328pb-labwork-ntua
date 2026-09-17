#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Two lines: the simplest possible reference for addressing both rows of
 * the 2x16 LCD. Static text, nothing dynamic -- just row 1, then a
 * cursor jump to row 2, then row 2. */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();
    lcd_clear_display();

    lcd_command(0x80); /* row 1, column 0 (DDRAM 0x00) -- lcd_init()
                         * already leaves the cursor here; explicit for
                         * clarity */
    lcd_string((const unsigned char *) "Line one!");

    lcd_command(0xC0); /* row 2, column 0 (DDRAM 0x40) -- the LCD's own
                         * auto-increment does NOT wrap here on its own,
                         * this jump is required to reach row 2 at all */
    lcd_string((const unsigned char *) "And line two.");

    while (1) {
        /* static text -- nothing left to do */
    }
}
