#define F_CPU 16000000UL
#include "../ntuaboard.h"
#include <stdio.h>

/* Ex7.2: show the DS18B20 reading on the LCD as a signed whole-degree
 * Celsius value, or "NO Device" if nothing answered. The original
 * submission (see ../../ex7-temp-sensor/walkthrough.md) read the sensor
 * with its own port of the given routines but only ever wrote the raw
 * value to PORTB in binary -- this is the LCD version 7.2 actually asks
 * for, using this library's therm_read_temperature(). */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();

    while (1) {
        uint16_t raw = therm_read_temperature();
        lcd_clear_display();

        if (raw == 0x8000) {
            lcd_string((const unsigned char *) "NO Device");
        } else {
            int16_t whole = (int16_t) raw >> 4; /* drop the 0.0625C fraction bits */
            char buf[8];
            sprintf(buf, "%d", whole);
            lcd_string((const unsigned char *) buf);
            lcd_data('C');
        }
        _delay_ms(1000);
    }
}
