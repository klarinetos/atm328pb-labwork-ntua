#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex4.3: simulated CO sensor (POT4/ADC0) driving a 6-LED bargraph, with
 * a "GAS DETECTED"/"CLEAR" LCD message once past the alarm threshold.
 * See ../../ex4-adc-lcd/walkthrough.md. */

#define GAS_THRESHOLD 51 /* ~1.0V of the 0-5V pot range, standing in for 75ppm */

int main(void) {
    DDRB = 0x3F;                            /* 6 bargraph LEDs */
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD */
    adc_init(0);
    lcd_init();

    uint8_t alarm = 0;

    while (1) {
        uint8_t z = adc_read8();

        if (!alarm) {
            uint8_t bank;
            if (z < 133) bank = 0b000111;
            else if (z < 174) bank = 0b001111;
            else if (z < 215) bank = 0b011111;
            else bank = 0b111111;
            PORTB = bank;
        } else {
            PORTB = 0x3F;
        }
        _delay_ms(300);
        PORTB = 0x00;
        _delay_ms(300);

        z = adc_read8();
        if (z < GAS_THRESHOLD) {
            if (alarm) {
                alarm = 0;
                lcd_clear_display();
                lcd_string((const unsigned char *) "CLEAR");
            }
        } else if (!alarm) {
            alarm = 1;
            lcd_clear_display();
            lcd_string((const unsigned char *) "GAS DETECTED");
        }
    }
}
