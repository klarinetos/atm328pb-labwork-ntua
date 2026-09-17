/* PDF pp. 13-14. Preserve executable formulas (comments disagree):
 * line 1: NOT(A NOT B + NOT B D), A..D on PB0..PB3.
 * line 2: NOT(NOT A + B + C + D), A..D on PC0..PC3. */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x0F);
    DDRC &= ~0x0F;
    PORTC |= 0x0F;
    exam_lcd_init();
    while (1) {
        uint8_t pb = (uint8_t)~PINB, pc = (uint8_t)~PINC;
        uint8_t a = pb & 1, b = (pb >> 1) & 1, d = (pb >> 3) & 1;
        uint8_t a1 = pc & 1, b1 = (pc >> 1) & 1;
        uint8_t c1 = (pc >> 2) & 1, d1 = (pc >> 3) & 1;
        lcd_command(0x80);
        lcd_data('0' + !((a && !b) || (!b && d)));
        lcd_command(0xC0);
        lcd_data('0' + !(!a1 || b1 || c1 || d1));
        _delay_ms(20);
    }
}
