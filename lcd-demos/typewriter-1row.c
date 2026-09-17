#define F_CPU 16000000UL
#include "ntuaboard.h"

/* Typewriter (single row): the simpler version of typewriter.c -- fills
 * only row 1, ignores further key presses once it's full (16 characters),
 * PB0 clears it. See typewriter.c for the two-row version, which adds a
 * cursor jump to continue onto row 2 once row 1 fills up. */

#define LCD_CAPACITY 16

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0); /* keypad rows/cols on IO1 */
    DDRB &= (uint8_t) ~(1 << PB0); /* PB0 as input... */
    PORTB |= (1 << PB0);           /* ...with its pull-up enabled */
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();

    uint8_t pos = 0;

    while (1) {
        if (!(PINB & (1 << PB0))) {
            lcd_clear_display(); /* also homes the cursor back to DDRAM 0x00 */
            pos = 0;
            while (!(PINB & (1 << PB0))) /* wait for release before continuing */
                ;
            _delay_ms(20); /* debounce */
        }

        uint8_t key = keypad_to_ascii();
        if (key && pos < LCD_CAPACITY) {
            lcd_data(key);
            pos++;
        }
    }
}
