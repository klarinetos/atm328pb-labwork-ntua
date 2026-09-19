#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex8.1: connect the ESP8266 and set its target URL, showing each step's
 * Success/Fail on the LCD as "1.Success"/"2.Fail" etc.
 * See ../../ex8-final-project/walkthrough.md. */

static void step(uint8_t n, const unsigned char *command) {
    unsigned char reply[32];
    usart_command(command, reply, sizeof reply);

    lcd_clear_display();
    lcd_data((uint8_t)('0' + n));
    lcd_data('.');
    if (reply[0] == '\0') /* usart_receive_str() timed out -- nothing answered */
        lcd_string((const unsigned char *) "TIMEOUT");
    else
        lcd_string(reply[0] == 'S' ? (const unsigned char *) "Success"
                                    : (const unsigned char *) "Fail");
    _delay_ms(2000);
}

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();
    USART_Init(MYUBRR);

    while (1) {
        step(1, (const unsigned char *) "ESP:connect");
        step(2, (const unsigned char *) "ESP:url:\"http://192.168.1.250:5000/data\"");
    }
}
