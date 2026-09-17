#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex6.1(e): '1'/'5'/'9'/'D' light PORTB bits 0-3 while held -- needs
 * level-detected keys (scan_keypad_pressed), not the edge-detected
 * keypad_to_ascii(). See ../../ex6-keypad/walkthrough.md.
 *
 * Bit positions below match codes[]'s order in ntuaboard.c:
 * {'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'} */

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    DDRB = 0xFF;

    while (1) {
        uint16_t keys = scan_keypad_pressed();
        uint8_t val = 0;
        if (keys & ((uint16_t) 1 << 0))  val |= 0x01; /* '1' */
        if (keys & ((uint16_t) 1 << 5))  val |= 0x02; /* '5' */
        if (keys & ((uint16_t) 1 << 10)) val |= 0x04; /* '9' */
        if (keys & ((uint16_t) 1 << 15)) val |= 0x08; /* 'D' */
        PORTB = val;
    }
}
