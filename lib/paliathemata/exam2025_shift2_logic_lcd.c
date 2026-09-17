/* PDF pp. 25-27: A..D=PB1..PB4 active-low, column 6, every 3 s.
 * F1=(!A & D) | (B & !C); F2=(B | !A) | !(!C | A). */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x1E);
    exam_lcd_init();
    while (1) {
        uint8_t input = ((uint8_t)~PINB >> 1) & 0x0F;
        uint8_t a = input & 1, b = (input >> 1) & 1;
        uint8_t c = (input >> 2) & 1, d = (input >> 3) & 1;
        uint8_t f1 = (!a && d) || (b && !c);
        uint8_t f2 = (b || !a) || !(!c || a);
        lcd_command(0x85);
        exam_bits_low_first(input, 4);
        lcd_command(0xC5);
        lcd_data('0' + f1);
        lcd_data('0' + f2);
        _delay_ms(3000);
    }
}
