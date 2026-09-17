#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex3.2: Timer1 PWM stepped by buttons (PB4 up / PB5 down) through the
 * same duty-cycle table as 3.1, plus a live ADC0 reading shown as an
 * 8-band bargraph on PORTD. See ../../ex3-adc-pwm/walkthrough.md. */

static const uint8_t duty_table[] = {5,   25,  46,  66,  87,  107, 127,
                                      148, 169, 189, 210, 230, 251};

int main(void) {
    pwm1_init();
    adc_init(0);

    DDRB = 0b001111; /* PB0-PB3 output (PB1 is the PWM pin; PB0,PB2,PB3
                       * unused, driven low so their LEDs don't float),
                       * PB4/PB5 input (buttons) -- pwm1_init() alone only
                       * sets up PB1 */
    PORTB = 0x00;
    DDRD = 0xFF; /* bargraph as output */

    uint8_t x = 6; /* start at ~50% duty */
    pwm1_set_duty(duty_table[x]);

    while (1) {
        if (!(PINB & (1 << PB4)) && x < 12) {
            x++;
            _delay_ms(50);
        }
        if (!(PINB & (1 << PB5)) && x > 0) {
            x--;
            _delay_ms(50);
        }
        pwm1_set_duty(duty_table[x]);

        uint8_t y = adc_read8();
        PORTD = (uint8_t)(1 << (y / 32)); /* one of 8 exclusive bands */

        _delay_ms(100);
    }
}
