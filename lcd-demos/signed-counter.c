#define F_CPU 16000000UL
#include "ntuaboard.h"
#include <avr/interrupt.h>
#include <stdio.h>

/* Signed counter on row 1: PB0 decrements, PB1 increments (regular
 * polled buttons, same pattern as every other demo here), and PB2
 * resets it back to zero via a genuine hardware interrupt.
 *
 * PB2 rather than PD2: PD2 is already the LCD's RS line (and the only
 * other pin wired to a real external interrupt, PD3/INT1, is the LCD's
 * E line) -- both of this chip's INT0/INT1 pins are unavailable while
 * the LCD is running. PB2 gets its own real onboard pushbutton (see the
 * board photo in ../docs/NtuaBoard_G1.pdf) and its own interrupt via a
 * Pin Change Interrupt (PCINT2) instead -- still genuinely
 * interrupt-driven, just through PCINT0_vect rather than INT0/INT1. */

static volatile int16_t count = 0;

ISR(PCINT0_vect) {
    if (PINB & (1 << PB2))
        return; /* PCINT0_vect fires on any PB0-PB7 change (incl. PB0/PB1,
                  * and PB2's own release) -- only act while PB2 reads low */

    /* Debounce (same algorithm ex2-interrupts uses for INT0/INT1): clear
     * the pending flag, wait ~5ms, re-check -- if it's set again a
     * bounce re-triggered the pin, so clear and wait again. */
    do {
        PCIFR |= (1 << PCIF0);
        _delay_ms(5);
    } while (PCIFR & (1 << PCIF0));

    if (!(PINB & (1 << PB2))) /* still pressed once the bouncing settles */
        count = 0;
}

int main(void) {
    DDRB &= (uint8_t) ~((1 << PB0) | (1 << PB1) | (1 << PB2)); /* inputs... */
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);              /* ...pull-ups */

    PCMSK0 = (1 << PCINT2); /* only PB2 wakes PCINT0_vect, not PB0/PB1 */
    PCICR = (1 << PCIE0);
    sei();

    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();

    int16_t shown = 1; /* != count's initial value, forces the first draw */
    char buf[7];        /* "-32768" + NUL */

    while (1) {
        if (!(PINB & (1 << PB0))) {
            cli();
            count--;
            sei(); /* count is also written by the ISR -- keep this
                     * read-modify-write from being interrupted mid-way */
            while (!(PINB & (1 << PB0)))
                ; /* wait for release before continuing */
            _delay_ms(20); /* debounce */
        }
        if (!(PINB & (1 << PB1))) {
            cli();
            count++;
            sei();
            while (!(PINB & (1 << PB1)))
                ;
            _delay_ms(20);
        }

        if (count != shown) {
            shown = count;
            lcd_command(0x80); /* row 1, column 0 */
            sprintf(buf, "%-6d", shown); /* space-padded: clears leftover
                                           * digits when the number shrinks
                                           * (e.g. -10 -> -9, or a reset
                                           * back down to 0) */
            for (uint8_t i = 0; buf[i]; i++)
                lcd_data((uint8_t) buf[i]);
        }
    }
}
