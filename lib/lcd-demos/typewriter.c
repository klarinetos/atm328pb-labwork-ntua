#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Typewriter: press keys on the 4x4 keypad to fill the 2x16 LCD
 * left-to-right, top row then bottom row, one character per key press.
 * PB0 clears the screen and starts over. Once both rows are full (32
 * characters), further key presses are ignored until PB0 resets it. */

#define LCD_COLS 16
#define LCD_ROWS 2
#define LCD_CAPACITY (LCD_COLS * LCD_ROWS)

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
            if (pos == LCD_COLS) {
                /* The LCD's own auto-increment walks off the visible
                 * window at the end of row 1 into invisible DDRAM
                 * (0x10-0x27) rather than wrapping onto row 2 -- jump
                 * the cursor to DDRAM 0x40 (row 2, column 0) by hand. */
                lcd_command(0xC0);
            }
            lcd_data(key);
            pos++;
        }
    }
}
