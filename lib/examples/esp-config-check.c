#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Diagnostic (not tied to a specific exercise): ask the ESP what it's
 * actually configured with right now -- "ESP:configuration" per the
 * protocol table in ex8-instructions.pdf -- and page through the reply
 * on the LCD. Answers "does this specific module still have the
 * documented default SSID/password, or was it reconfigured to
 * something else at some point?" without guessing.
 *
 * The reply turns out to be multiple lines ("ESP8266: Configuration:"
 * followed by the actual SSID/password/etc, each its own line) -- this
 * reads and displays them one at a time via repeated
 * usart_receive_str() calls, rather than a single usart_command()
 * (which only reads the *first* line, silently dropping the rest). */

int main(void) {
    DDRD |= (1 << PD2) | (1 << PD3) | 0xF0;
    lcd_init();
    USART_Init(MYUBRR);

    while (1) {
        usart_transmit_str((const unsigned char *) "ESP:configuration");

        while (1) {
            unsigned char line[40];
            usart_receive_str(line, sizeof line);
            if (line[0] == '\0')
                break; /* ~3s of silence -- assume the reply is done */

            uint8_t len = 0;
            while (line[len])
                len++;

            lcd_clear_display();
            for (uint8_t i = 0; i < 32 && i < len; i++) {
                if (i == 16)
                    lcd_command(0xC0);
                lcd_data(line[i]);
            }
            _delay_ms(3000);
        }

        _delay_ms(1000); /* brief pause, then re-query and cycle again */
    }
}
