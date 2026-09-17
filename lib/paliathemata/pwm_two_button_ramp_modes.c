/* PDF pp. 17-19: PC1 selects 10..90..10%, 500 ms/step;
 * PC0 selects 10..90%, then reset to 10%, 100 ms/step.
 * Poll buttons every 10 ms instead of waiting an entire ramp. */
#include "exam_helpers.h"
int main(void) {
    uint8_t mode = 1, duty = 10, previous = 0, stable = 0, ticks = 0;
    int8_t direction = 1;
    DDRC &= ~0x03;
    PORTC |= 0x03;
    pwm1_init();
    TCCR1B = (1 << WGM12) | (1 << CS12); /* Source prescaler /256. */
    pwm1_set_duty((uint16_t)255 * duty / 100);
    while (1) {
        _delay_ms(10);
        uint8_t buttons = (uint8_t)~PINC & 3;
        if (buttons == previous && buttons != stable) {
            stable = buttons;
            uint8_t selected = buttons == 2 ? 1 : buttons == 1 ? 2 : 0;
            if (selected && selected != mode) {
                mode = selected; duty = 10; direction = 1; ticks = 0;
                pwm1_set_duty((uint16_t)255 * duty / 100);
            }
        }
        previous = buttons;
        if (++ticks < (mode == 1 ? 50 : 10)) continue;
        ticks = 0;
        if (mode == 2) duty = duty == 90 ? 10 : duty + 10;
        else {
            if (duty == 90) direction = -1;
            if (duty == 10) direction = 1;
            duty = (uint8_t)(duty + direction * 10);
        }
        pwm1_set_duty((uint16_t)255 * duty / 100);
    }
}
