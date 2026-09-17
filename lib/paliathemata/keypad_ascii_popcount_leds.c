/* PDF pp. 16-17, shift 4: '#' lights PC0 for 1 s; ASCII bit count on PB.
 * The PDF subtracts one TWICE. Preserve that behavior explicitly; set
 * ASCII_COUNT_OFFSET=0 for a plain count, or 1 for a single subtraction. */
#include "exam_helpers.h"
#ifndef ASCII_COUNT_OFFSET
#define ASCII_COUNT_OFFSET 2
#endif
int main(void) {
    DDRB |= 0x3F;
    DDRC |= (1 << PC0); /* Leave PC4/PC5 for TWI. */
    PORTB &= ~0x3F;
    PORTC &= ~(1 << PC0);
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    PCA9555_0_write(REG_OUTPUT_1, 0xFF);
    while (1) {
        uint8_t key = keypad_to_ascii();
        if (!key) continue;
        if (key == '#') {
            PORTC |= (1 << PC0);
            _delay_ms(1000);
            PORTC &= ~(1 << PC0);
        }
        uint8_t count = exam_popcount(key);
        uint8_t value = count >= ASCII_COUNT_OFFSET ? count - ASCII_COUNT_OFFSET : 0;
        PORTB = (PORTB & ~0x3F) | (value & 0x3F);
    }
}
