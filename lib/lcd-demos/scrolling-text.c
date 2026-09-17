#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Scrolling text: writes a message longer than the visible 16 columns
 * into row 1's DDRAM, then slides the display window back and forth
 * over it using the controller's own "shift display" command -- no
 * buttons, no re-writing characters, just watch it scroll. */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0; /* LCD control + data lines --
                                              * lcd_init() doesn't set this
                                              * itself, see ntuaboard.h */
    lcd_init();
    lcd_clear_display();

    const char msg[] = "NTUABOARD - MICROLAB - AVR - ";
    for (uint8_t i = 0; msg[i]; i++) {
        lcd_data((uint8_t) msg[i]);
    }

    const uint8_t max_shift = (uint8_t)(sizeof(msg) - 1 - 16); /* stop once the tail's in view */
    uint8_t shifted = 0;

    while (1) {
        _delay_ms(400);
        if (shifted < max_shift) {
            lcd_command(0x18); /* shift entire display left */
            shifted++;
        } else {
            for (uint8_t i = 0; i < max_shift; i++) {
                lcd_command(0x1C); /* shift entire display right */
                _delay_ms(400);
            }
            shifted = 0;
        }
    }
}
