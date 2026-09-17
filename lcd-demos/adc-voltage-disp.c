#define F_CPU 16000000UL
#include "ntuaboard.h"

/* ADC voltage display: reads a voltage on ADC2 once a second and shows
 * it on the LCD as "D.DD V", centered. Adapted from lib/examples/4.2.c
 * (the Ex4.2 rewrite), with extra comments here since this one's meant
 * to be read as a reference rather than just run.
 *
 * How the reading becomes a voltage: adc_read8() returns the ADC's top
 * 8 bits (0-255), which for a 0-5V input maps linearly across that whole
 * range. Scaling that up to hundredths of a volt (0-500, so "500" means
 * 5.00V) avoids floating point entirely -- it's all integer math:
 *
 *   centivolts = raw * 500 / 255
 *
 * Since the board's ADC reference is 5V, centivolts never exceeds 500,
 * so the integer part of the voltage is always exactly one digit --
 * which is what makes "D.DD V" a fixed 6 characters every time, and is
 * why the centering below can be a constant column offset instead of
 * something that has to measure the string first.
 */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    adc_init(2); /* ADC channel 2 */
    lcd_init();

    while (1) {
        uint8_t raw = adc_read8();
        uint16_t centivolts = (uint16_t)((uint32_t) raw * 500 / 255);

        lcd_clear_display();
        /* "D.DD V" is always exactly 6 characters; center it on the
         * 16-column display with (16-6)/2 = 5 columns of left padding,
         * via a "set DDRAM address" command: 0x80 = row 1, column 0,
         * +5 shifts the cursor 5 columns right before writing anything. */
        lcd_command(0x85);
        lcd_data((uint8_t)('0' + centivolts / 100));       /* the one units digit */
        lcd_data('.');
        lcd_data((uint8_t)('0' + (centivolts / 10) % 10)); /* tenths */
        lcd_data((uint8_t)('0' + centivolts % 10));        /* hundredths */
        lcd_data(' ');
        lcd_data('V');

        _delay_ms(1000);
    }
}
