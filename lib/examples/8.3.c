#define F_CPU 16000000UL
#include "../ntuaboard.h"
#include <stdio.h>
#include <stdlib.h>

/* Ex8.3: send temperature+pressure+status to the Gateway as JSON, show
 * its reply on the LCD as step "4.". See
 * ../../ex8-final-project/walkthrough.md.
 *
 * Unlike the original submission -- where the "NURSE CALL" status and a
 * keypad_to_ascii() call both existed but nothing ever wired them
 * together, so that status was dead code -- this version actually reads
 * the keypad: the key matching this team's last digit ('9') sets it,
 * '#' clears it back to OK. */

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();
    USART_Init(MYUBRR);
    adc_init(0);

    uint8_t nurse_call = 0;

    while (1) {
        unsigned char reply[64];

        lcd_clear_display();
        lcd_data('1');
        lcd_data('.');
        usart_command((const unsigned char *) "ESP:connect", reply, sizeof reply);

        lcd_clear_display();
        lcd_data('2');
        lcd_data('.');
        usart_command((const unsigned char *) "ESP:url:\"http://192.168.1.250:5000/data\"",
                      reply, sizeof reply);

        uint16_t raw = therm_read_temperature();
        int16_t temp_tenths = (int16_t)(((int32_t) raw * 10) >> 4) + 120;
        uint8_t pressure_raw = adc_read8();
        uint16_t pressure_tenths = (uint16_t)(((uint32_t) pressure_raw * 200) / 255);

        uint8_t key = keypad_to_ascii();
        if (key == '9')
            nurse_call = 1;
        else if (key == '#')
            nurse_call = 0;

        const char *status = "OK";
        if (nurse_call)
            status = "NURSE CALL";
        else if (temp_tenths < 340 || temp_tenths > 370)
            status = "CHECK TEMP";
        else if (pressure_raw < 51 || pressure_raw > 153)
            status = "CHECK PRESSURE";

        char payload[160];
        sprintf(payload,
                "ESP:payload:[{\"name\":\"temperature\",\"value\":\"%d.%d\"},"
                "{\"name\":\"pressure\",\"value\":\"%d.%d\"},"
                "{\"name\":\"team\",\"value\":\"59\"},"
                "{\"name\":\"status\",\"value\":\"%s\"}]",
                temp_tenths / 10, abs(temp_tenths % 10), pressure_tenths / 10,
                pressure_tenths % 10, status);

        lcd_clear_display();
        lcd_data('3');
        lcd_data('.');
        usart_command((const unsigned char *) payload, reply, sizeof reply);

        lcd_clear_display();
        lcd_data('4');
        lcd_data('.');
        usart_command((const unsigned char *) "ESP:transmit", reply, sizeof reply);
        lcd_string(reply);

        _delay_ms(2000);
    }
}
