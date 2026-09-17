/* PDF pp. 19-20, shift 5: INT1/PD3 rising edge, ADC1/PC1.
 * PB5 plus PB0 (>=1600 mV) or PB1 (<1600 mV) for 2 seconds.
 * ISR only latches the request; conversions/delays run in main. */
#include "exam_helpers.h"
#include <avr/interrupt.h>
static volatile uint8_t pending;
ISR(INT1_vect) { EIMSK &= ~(1 << INT1); pending = 1; }
int main(void) {
    DDRB |= 0x23;
    PORTB &= ~0x23;
    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);
    exam_adc10_init();
    EICRA = (1 << ISC11) | (1 << ISC10);
    EIFR = (1 << INTF1);
    EIMSK = (1 << INT1);
    sei();
    while (1) {
        if (!pending) continue;
        pending = 0;
        PORTB |= (1 << PB5);
        uint16_t raw = exam_adc10_read();
        /* PDF uses raw*5000/1024. Compare without float/truncation. */
        uint8_t led = (uint32_t)raw * 5000 >= 1600UL * 1024 ? 1 : 2;
        PORTB = (PORTB & ~0x23) | 0x20 | led;
        _delay_ms(2000);
        PORTB &= ~0x23;
        EIFR = (1 << INTF1); /* Discard bounce/events during the display. */
        EIMSK |= (1 << INT1);
    }
}
