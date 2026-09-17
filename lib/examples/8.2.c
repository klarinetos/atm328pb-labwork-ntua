#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex8.2(a,b): add the DS18B20 + POT0 readings and an OK/CHECK TEMP/CHECK
 * PRESSURE status, shown on the LCD. See
 * ../../ex8-final-project/walkthrough.md -- the keypad-driven "NURSE
 * CALL" status from 8.2(c) is wired up in 8.3.c instead, alongside
 * sending everything to the Gateway. */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();
    adc_init(0);

    while (1) {
        uint16_t raw = therm_read_temperature();
        /* raw is in 0.0625C units (temp*16); scale to tenths of a
         * degree (temp*10) and offset ~12C to read near a real patient
         * temperature, matching the original's simulated setup. */
        int16_t temp_tenths = (int16_t)(((int32_t) raw * 10) >> 4) + 120;

        uint8_t pressure_raw = adc_read8();

        const char *status = "OK";
        if (temp_tenths < 340 || temp_tenths > 370)
            status = "CHECK TEMP";
        else if (pressure_raw < 51 || pressure_raw > 153)
            status = "CHECK PRESSURE";

        lcd_clear_display();
        lcd_string((const unsigned char *) status);
        _delay_ms(2000);
    }
}
