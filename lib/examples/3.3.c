#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex3.3: two brightness-control modes for the same PWM output -- mode 1
 * by the PB4/PB5 buttons (as in 3.1/3.2), mode 2 directly by POT1 (ADC0).
 * See ../../ex3-adc-pwm/walkthrough.md. */

static const uint8_t duty_table[] = {5,   25,  46,  66,  87,  107, 127,
                                      148, 169, 189, 210, 230, 251};

int main(void) {
    pwm1_init();
    adc_init(0);
    DDRD = 0x00; /* mode-select + duty buttons are all on PORTD here */
    DDRB = 0xFF; /* pwm1_init() only sets PB1 (the PWM pin) as output --
                  * drive the rest of PORTB low too, or its other LEDs
                  * float and light up from noise. */
    PORTB = 0x00;

    uint8_t dc_value = 6;
    uint8_t mode = 1;
    pwm1_set_duty(duty_table[dc_value]);

    while (1) {
        if (!(PIND & (1 << PD6))) {
            _delay_ms(50);
            mode = 1;
        }
        if (!(PIND & (1 << PD7))) {
            _delay_ms(50);
            mode = 2;
        }

        if (mode == 1) {
            if (!(PIND & (1 << PD1)) && dc_value < 12) {
                dc_value++;
                _delay_ms(50);
            }
            if (!(PIND & (1 << PD2)) && dc_value > 0) {
                dc_value--;
                _delay_ms(50);
            }
            pwm1_set_duty(duty_table[dc_value]);
        } else {
            pwm1_set_duty(adc_read8());
            _delay_ms(20);
        }
    }
}
