#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex4.2: poll the ADC and print the resulting voltage on the LCD to two
 * decimal places. See ../../ex4-adc-lcd/walkthrough.md. */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines */
    adc_init(2); /* ADC2, matching the original 4.1/4.2 wiring */
    lcd_init();

    while (1) {
        uint8_t raw = adc_read8();
        /* raw is 0-255 for 0-5V; scale to hundredths of a volt (0-500). */
        uint16_t centivolts = (uint16_t)((uint32_t) raw * 500 / 255);

        lcd_clear_display();
        lcd_data((uint8_t)('0' + centivolts / 100));
        lcd_data(',');
        lcd_data((uint8_t)('0' + (centivolts / 10) % 10));
        lcd_data((uint8_t)('0' + centivolts % 10));

        _delay_ms(1000);
    }
}
