/* PDF pp. 9-10, shift 1.2: F = NOT(A NOT B + NOT B D).
 * A=PB0, B=PB1, D=PB3, active-low buttons. C is unused. */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x0F);
    exam_lcd_init();
    while (1) {
        uint8_t in = (uint8_t)~PINB;
        uint8_t a = in & 1, b = (in >> 1) & 1, d = (in >> 3) & 1;
        lcd_command(0x80);
        lcd_data('0' + !((a && !b) || (!b && d)));
        _delay_ms(20);
    }
}
