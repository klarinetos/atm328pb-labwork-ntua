/* PDF pp. 10-11: number of pressed PB0..PB5 buttons, decimal 0..6.
 * Correct the source's count-1: the heading asks for the actual count. */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x3F);
    exam_lcd_init();
    while (1) {
        lcd_command(0x80);
        lcd_data('0' + exam_popcount((uint8_t)~PINB & 0x3F));
        _delay_ms(20);
    }
}
