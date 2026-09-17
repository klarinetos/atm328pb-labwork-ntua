#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Build variants: a_b (inputs), c_d (keys), full (all four questions). */
#ifndef ENABLE_INPUTS
#define ENABLE_INPUTS 1
#endif
#ifndef ENABLE_KEYS
#define ENABLE_KEYS 1
#endif

#if ENABLE_INPUTS
volatile uint8_t dig_inp;

static void sample_inputs(void) {
    /* PB2 -> bit 4, PB3 -> bit 5, PB4 -> bit 6, PB5 -> bit 7. */
    dig_inp = (uint8_t)((PINB & 0x3C) << 2);
    if (dig_inp == 0)
        PORTC |= (1 << PC0);
    else
        PORTC &= ~(1 << PC0);

    lcd_command(0x80); /* First line: PB2 PB3 PB4 PB5, left to right. */
    for (uint8_t bit = 4; bit < 8; ++bit)
        lcd_data((dig_inp & (1 << bit)) ? '1' : '0');
}
#endif

#if ENABLE_KEYS
/* Reserve these registers in BOTH translation units using the Makefile.
 * R17 = previous key, R18 = newest key. No added library may clobber them. */
register uint8_t previous_key asm("r17");
register uint8_t newest_key asm("r18");
static uint8_t key_count;

static void poll_keys(void) {
    uint8_t key = keypad_to_ascii(); /* Debounced press edge, not held level. */
    if (!key)
        return;

    previous_key = newest_key;
    newest_key = key;
    if (key_count < 2)
        ++key_count;

    /* Until two keys have been pressed there is no pair to compare. */
    if (key_count == 2 && previous_key == newest_key)
        PORTC |= (1 << PC1);
    else
        PORTC &= ~(1 << PC1);

    lcd_command(0xC0); /* Second line: older character, newer character. */
    lcd_data(key_count == 2 ? previous_key : ' ');
    lcd_data(newest_key);
}
#endif

int main(void) {
    DDRD |= 0xFC; /* LCD: PD2=RS, PD3=E, PD4..PD7=data. */
    lcd_init();
    lcd_clear_display();

#if ENABLE_INPUTS
    DDRB &= ~0x3C;
    PORTB |= 0x3C; /* Pull-ups: open inputs read 1, switches to GND read 0. */
    PORTC &= ~(1 << PC0);
    DDRC |= (1 << PC0);
#endif
#if ENABLE_KEYS
    PORTC &= ~(1 << PC1);
    DDRC |= (1 << PC1);
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    PCA9555_0_write(REG_OUTPUT_1, 0xFF);
    previous_key = 0;
    newest_key = 0;
#endif
#if ENABLE_INPUTS
    /* 16 MHz / 1024 = 15625 Hz; 46875 ticks = 3 seconds.
     * Poll the compare flag instead of blocking keypad scans for 3 seconds. */
    TCCR1A = 0;
    TCCR1B = 0;
    TIMSK1 = 0;
    TCNT1 = 0;
    OCR1A = 46874;
    TIFR1 = (1 << OCF1A);
    sample_inputs(); /* Initial display immediately, then every 3 seconds. */
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
#endif

    while (1) {
#if ENABLE_INPUTS
        if (TIFR1 & (1 << OCF1A)) {
            TIFR1 = (1 << OCF1A); /* Clear flag by writing one. */
            sample_inputs();
        }
#endif
#if ENABLE_KEYS
        poll_keys();
#endif
    }
}
