#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex6.3: 2-digit "team number" combination lock on the keypad --
 * 6 LEDs (PORTB) on for 3s on a correct match, blinking for 6s on a
 * wrong one. See ../../ex6-keypad/walkthrough.md. */

static const char team[2] = {'5', '9'}; /* team 59, first digit then second */

static void char_press(uint8_t c) {
    static char input[2] = {0, 0};
    input[1] = input[0];
    input[0] = (char) c;
    if (!input[1])
        return; /* still need a second digit */

    if (input[1] == team[0] && input[0] == team[1]) {
        PORTB = 0xFF;
        _delay_ms(3000);
    } else {
        for (uint8_t i = 0; i < 12; i++) {
            PORTB = 0xFF;
            _delay_ms(250);
            PORTB = 0x00;
            _delay_ms(250);
        }
    }
    PORTB = 0x00;
    input[0] = input[1] = 0;
}

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    DDRB = 0xFF;

    while (1) {
        uint8_t val = keypad_to_ascii();
        if (val)
            char_press(val);
    }
}
