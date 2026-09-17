/* PDF pp. 8-9, shift 1.1: lowest pressed PB0..PB3 -> LCD 1..4, none -> 0. */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x0F);
    exam_lcd_init();
    while (1) {
        uint8_t pressed = (uint8_t)~PINB & 0x0F;
        uint8_t position = 0;
        if (pressed) {
            position = 1;
            while (!(pressed & 1)) { pressed >>= 1; ++position; }
        }
        lcd_command(0x80);
        lcd_data('0' + position);
        _delay_ms(20);
    }
}
