#define F_CPU 16000000UL
#include "../ntuaboard.h"
#include <stdbool.h>

/* Ex8.2(a,b): add the DS18B20 + POT0 readings and an OK/CHECK TEMP/CHECK
 * PRESSURE status, shown on the LCD. See
 * ../../ex8-final-project/walkthrough.md -- the keypad-driven "NURSE
 * CALL" status from 8.2(c) is wired up in 8.3.c instead, alongside
 * sending everything to the Gateway. */

enum warning_t {temperature, pressure};

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();
    adc_init(0);

    while (1) {
        uint16_t raw = therm_read_temperature();
        lcd_clear_display();

        if (raw == 0x8000) {
            /* No device answered the 1-Wire reset -- without this check
             * that sentinel value would otherwise get folded straight
             * into the temperature math below and land wildly out of
             * range, permanently showing "CHECK TEMP" regardless of
             * what's actually happening. */
            lcd_string((const unsigned char *) "NO Device");
        } else {
            /* raw is in 0.0625C units (temp*16); scale to tenths of a
             * degree (temp*10) and offset it to read near a real
             * patient temperature, matching the original's simulated
             * setup. +9.0C is tuned for ~27C room temperature (this
             * board's actual test conditions), landing at 36.0C --
             * comfortably inside the healthy 34.0-37.0C window with
             * room on both sides, instead of the original's fixed
             * +12C (tuned for a cooler ~22-25C room), which pushed a
             * 27C room to 39.0C -- always out of range. */
            int16_t temp_tenths = (int16_t)(((int32_t) raw * 10) >> 4) + 90;
            uint8_t pressure_raw = adc_read8();

            bool warning[2];
            warning[temperature] = temp_tenths < 340 || temp_tenths > 370;
            warning[pressure] = pressure_raw < 51 || pressure_raw > 153;

            /* One warning on its own goes on row 1 alone; with both
             * active there's no room for "OK" so each gets its own
             * row instead -- temperature first, pressure second. */
            if (warning[temperature])
                lcd_string((const unsigned char *) "CHECK TEMP");
            else if (warning[pressure])
                lcd_string((const unsigned char *) "CHECK PRESSURE");
            else
                lcd_string((const unsigned char *) "OK");

            if (warning[temperature] && warning[pressure]) {
                lcd_command(0xC0); /* row 2, column 0 */
                lcd_string((const unsigned char *) "CHECK PRESSURE");
            }
        }

        _delay_ms(2000);
    }
}
