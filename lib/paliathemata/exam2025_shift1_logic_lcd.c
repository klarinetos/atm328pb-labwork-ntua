/* PDF pp. 22-25: PB0..PB3 active-low, LCD column 3, every 5 s.
 * Preserve code formulas where PDF comments disagree:
 * F1 = (!B | D) | (C & !A); F2 = (A & !B) | !(!A | D). */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x0F);
    exam_lcd_init();
    while (1) {
        uint8_t input = (uint8_t)~PINB & 0x0F;
        uint8_t a = input & 1, b = (input >> 1) & 1;
        uint8_t c = (input >> 2) & 1, d = (input >> 3) & 1;
        uint8_t f1 = (!b || d) || (c && !a);
        uint8_t f2 = (a && !b) || !(!a || d);
        lcd_command(0x82);
        exam_bits_low_first(input, 4);
        lcd_command(0xC2);
        lcd_data('0' + f1);
        lcd_data('0' + f2);
        _delay_ms(5000);
    }
}
