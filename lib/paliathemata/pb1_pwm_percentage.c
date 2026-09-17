/* PDF pp. 15-16: PB1 PWM with configurable percentage, example 20%.
 * Preserve the source's /8 prescaler (7812.5 Hz at 16 MHz). */
#include "exam_helpers.h"
#ifndef DUTY_PERCENT
#define DUTY_PERCENT 20
#endif
#if DUTY_PERCENT < 0 || DUTY_PERCENT > 100
#error DUTY_PERCENT must be between 0 and 100
#endif
int main(void) {
    pwm1_init();
    TCCR1B = (1 << WGM12) | (1 << CS11);
    pwm1_set_duty((uint16_t)255 * DUTY_PERCENT / 100);
    /* Fast PWM OCR=0 has a narrow pulse: disconnect for exact 0%. */
    if (DUTY_PERCENT == 0 || DUTY_PERCENT == 100) {
        TCCR1A &= ~(1 << COM1A1);
        if (DUTY_PERCENT == 100) PORTB |= (1 << PB1);
        else PORTB &= ~(1 << PB1);
    }
    while (1) {}
}
