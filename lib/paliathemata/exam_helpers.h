#ifndef EXAM_HELPERS_H
#define EXAM_HELPERS_H
#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Small formatting helpers only; peripheral drivers remain in ntuaboard.c. */
static inline void exam_lcd_init(void) {
    DDRD |= 0xFC;
    lcd_init();
    lcd_clear_display();
}

static inline void exam_pb_inputs(uint8_t mask) {
    DDRB &= (uint8_t)~mask;
    PORTB |= mask; /* Buttons to GND, active low. */
}

static inline uint8_t exam_popcount(uint8_t value) {
    uint8_t count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}

static inline void exam_binary8(uint8_t value) {
    for (uint8_t mask = 0x80; mask; mask >>= 1)
        lcd_data((value & mask) ? '1' : '0');
}

static inline void exam_bits_low_first(uint8_t value, uint8_t count) {
    while (count--) {
        lcd_data('0' + (value & 1));
        value >>= 1;
    }
}

/* The existing ADC read helper is 8-bit. Keep 10-bit precision for the
 * voltage thresholds, reusing adc_init but switching to right adjustment. */
static inline void exam_adc10_init(void) {
    DDRC &= ~(1 << PC1);
    PORTC &= ~(1 << PC1);
    adc_init(1);
    ADMUX &= ~(1 << ADLAR);
    DIDR0 |= (1 << ADC1D);
}

static inline uint16_t exam_adc10_read(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)) {}
    return ADC;
}
#endif
