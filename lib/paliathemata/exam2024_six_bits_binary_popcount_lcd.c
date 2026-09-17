/* PDF pp. 20-22: PB0..PB5 -> bits 7..2 every 3.5 s.
 * Both LCD lines begin at column 5; line 2 = popcount + 0x1E. */
#include "exam_helpers.h"
int main(void) {
    exam_pb_inputs(0x3F);
    exam_lcd_init();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 54687; /* 3.500032 s at 16 MHz /1024. */
    TIFR1 = (1 << OCF1A);
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
    while (1) {
        uint8_t input = (PINB & 0x3F) << 2; /* Raw levels, no inversion. */
        lcd_command(0x84);
        exam_binary8(input);
        lcd_command(0xC4);
        exam_binary8(exam_popcount(input) + 0x1E);
        while (!(TIFR1 & (1 << OCF1A))) {}
        TIFR1 = (1 << OCF1A);
    }
}
