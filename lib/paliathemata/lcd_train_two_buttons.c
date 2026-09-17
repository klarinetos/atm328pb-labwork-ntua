/* PDF pp. 14-15: PB0 starts left->right, PB1 right->left.
 * Only the button at the current departure endpoint is accepted.
 * Rewrite the old cell and new cell, avoiding HD44780 display-shift wrap. */
#include "exam_helpers.h"
int main(void) {
    uint8_t column = 0;
    exam_pb_inputs(0x03);
    exam_lcd_init();
    lcd_data('O');
    while (1) {
        uint8_t button = column == 0 ? 1 : 2;
        if (((uint8_t)~PINB & 0x03) != button) continue;
        _delay_ms(20);
        if (((uint8_t)~PINB & 0x03) != button) continue;
        int8_t direction = column == 0 ? 1 : -1;
        for (uint8_t step = 0; step < 15; ++step) {
            _delay_ms(1000);
            lcd_command(0x80 + column);
            lcd_data(' ');
            column = (uint8_t)(column + direction);
            lcd_command(0x80 + column);
            lcd_data('O');
        }
    }
}
