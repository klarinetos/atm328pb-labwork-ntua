#define F_CPU 16000000UL
#include "../ntuaboard.h"
#include <stdio.h>
#include <string.h>

/* Ex7.2: show the DS18B20 reading on the LCD as a signed whole-degree
 * Celsius value on row 1 (converted to Fahrenheit on row 2), both
 * centered with the LCD's built-in degree glyph (0xDF in this
 * controller's character ROM), or "NO Device" if nothing answered. The
 * original submission (see ../../ex7-temp-sensor/walkthrough.md) read
 * the sensor with its own port of the given routines but only ever
 * wrote the raw value to PORTB in binary -- this is the LCD version
 * 7.2 actually asks for, using this library's therm_read_temperature(). */

static void lcd_write_centered(uint8_t row, const char *str) {
    uint8_t len = (uint8_t) strlen(str);
    if (len > 16)
        len = 16;
    uint8_t pad = (uint8_t) ((16 - len) / 2);
    lcd_command((uint8_t) ((row == 0 ? 0x80 : 0xC0) + pad));
    while (*str)
        lcd_data((uint8_t) *str++);
}

/* Formats "<n><degree><unit>" into buf and centers it on the given row. */
static void show_temp(uint8_t row, int16_t n, char unit) {
    char buf[10];
    sprintf(buf, "%d", n);
    uint8_t i = (uint8_t) strlen(buf);
    buf[i] = '\xDF'; /* degree symbol */
    buf[i + 1] = unit;
    buf[i + 2] = '\0';
    lcd_write_centered(row, buf);
}

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();

    while (1) {
        uint16_t raw = therm_read_temperature();
        lcd_clear_display();

        if (raw == 0x8000) {
            lcd_write_centered(0, "NO Device");
        } else {
            int16_t celsius = (int16_t) raw >> 4; /* drop the 0.0625C fraction bits */
            int16_t fahrenheit = (int16_t) (celsius * 9 / 5 + 32);
            show_temp(0, celsius, 'C');
            show_temp(1, fahrenheit, 'F');
        }
        _delay_ms(1000);
    }
}
