/* PDF pp. 27-29: INT0 rising edge, ADC1; PB5 busy for 2 s,
 * then PB3 indicates >=1450 mV until the next request.
 * Correct INT0 input to PD2 (the PDF initializes PD3 instead). */
#include "exam_helpers.h"
#include <avr/interrupt.h>
static volatile uint8_t pending;
ISR(INT0_vect) { EIMSK &= ~(1 << INT0); pending = 1; }
int main(void) {
    DDRB |= (1 << PB3) | (1 << PB5);
    PORTB &= ~((1 << PB3) | (1 << PB5));
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);
    exam_adc10_init();
    EICRA = (1 << ISC01) | (1 << ISC00);
    EIFR = (1 << INTF0);
    EIMSK = (1 << INT0);
    sei();
    while (1) {
        if (!pending) continue;
        pending = 0;
        PORTB |= (1 << PB5);
        uint16_t raw = exam_adc10_read();
        _delay_ms(2000);
        PORTB &= ~((1 << PB3) | (1 << PB5));
        /* Preserve this example's /1023 voltage scale. */
        if ((uint32_t)raw * 5000 >= 1450UL * 1023) PORTB |= (1 << PB3);
        EIFR = (1 << INTF0);
        EIMSK |= (1 << INT0);
    }
}
