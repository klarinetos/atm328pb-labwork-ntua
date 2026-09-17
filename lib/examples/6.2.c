#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex6.2: show the most recently pressed key on the LCD.
 * See ../../ex6-keypad/walkthrough.md. */

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();

    uint8_t prev = 0;
    while (1) {
        uint8_t val = keypad_to_ascii();
        if (val && val != prev) {
            prev = val;
            lcd_clear_display();
            lcd_data(val);
        }
    }
}
