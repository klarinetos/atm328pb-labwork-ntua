/*
 * ntuaboard.c — implementation of ntuaboard.h.
 * See ntuaboard.h for what each function does and where it came from.
 */

#include "ntuaboard.h"
#include <avr/io.h>
#include <util/delay.h>

/* ======================================================================
 * TWI (I2C) master
 * ====================================================================== */

#define TWBR0_VALUE (((F_CPU) / SCL_CLOCK - 16) / 2)

#define TW_START         0x08
#define TW_REP_START     0x10
#define TW_MT_SLA_ACK    0x18
#define TW_MT_SLA_NACK   0x20
#define TW_MT_DATA_ACK   0x28
#define TW_MR_SLA_ACK    0x40
#define TW_MR_SLA_NACK   0x48
#define TW_MR_DATA_NACK  0x58

#define TW_STATUS_MASK  0b11111000
#define TW_STATUS       (TWSR0 & TW_STATUS_MASK)

void twi_init(void) {
    TWSR0 = 0;               /* PRESCALER_VALUE = 1 */
    TWBR0 = TWBR0_VALUE;
}

uint8_t twi_start(uint8_t address) {
    uint8_t twi_status;

    TWCR0 = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)))
        ;

    twi_status = TW_STATUS & 0xF8;
    if (twi_status != TW_START && twi_status != TW_REP_START)
        return 1;

    TWDR0 = address;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)))
        ;

    twi_status = TW_STATUS & 0xF8;
    if (twi_status != TW_MT_SLA_ACK && twi_status != TW_MR_SLA_ACK)
        return 1;

    return 0;
}

void twi_start_wait(uint8_t address) {
    uint8_t twi_status;
    while (1) {
        TWCR0 = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
        while (!(TWCR0 & (1 << TWINT)))
            ;

        twi_status = TW_STATUS & 0xF8;
        if (twi_status != TW_START && twi_status != TW_REP_START)
            continue;

        TWDR0 = address;
        TWCR0 = (1 << TWINT) | (1 << TWEN);
        while (!(TWCR0 & (1 << TWINT)))
            ;

        twi_status = TW_STATUS & 0xF8;
        if (twi_status == TW_MT_SLA_NACK || twi_status == TW_MR_DATA_NACK) {
            TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
            while (TWCR0 & (1 << TWSTO))
                ;
            continue;
        }
        break;
    }
}

uint8_t twi_rep_start(uint8_t address) {
    return twi_start(address);
}

void twi_stop(void) {
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    while (TWCR0 & (1 << TWSTO))
        ;
}

uint8_t twi_write(uint8_t data) {
    TWDR0 = data;
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)))
        ;
    if ((TW_STATUS & 0xF8) != TW_MT_DATA_ACK)
        return 1;
    return 0;
}

uint8_t twi_readAck(void) {
    TWCR0 = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR0 & (1 << TWINT)))
        ;
    return TWDR0;
}

uint8_t twi_readNak(void) {
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR0 & (1 << TWINT)))
        ;
    return TWDR0;
}

/* ======================================================================
 * PCA9555
 * ====================================================================== */

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value) {
    twi_start_wait(PCA9555_0_ADDRESS + 0 /* TWI_WRITE */);
    twi_write(reg);
    twi_write(value);
    twi_stop();
}

uint8_t PCA9555_0_read(PCA9555_REGISTERS reg) {
    uint8_t ret_val;
    twi_start_wait(PCA9555_0_ADDRESS + 0 /* TWI_WRITE */);
    twi_write(reg);
    twi_rep_start(PCA9555_0_ADDRESS + 1 /* TWI_READ */);
    ret_val = twi_readNak();
    twi_stop();
    return ret_val;
}

/* ======================================================================
 * 4x4 keypad
 * ====================================================================== */

uint8_t scan_row(uint8_t row) {
    uint8_t temp = 0x01;
    temp = temp << (row - 1);
    PCA9555_0_write(REG_OUTPUT_1, (uint8_t) ~temp); /* drive this row low */
    temp = PCA9555_0_read(REG_INPUT_1);
    temp = (uint8_t) ~temp;
    temp = temp >> 4; /* columns are the upper nibble */
    return temp;
}

uint16_t scan_keypad(void) {
    uint16_t result = 0x00;
    for (uint8_t i = 1; i < 5; i++) {
        result = result << 4;
        result += scan_row(i);
    }
    return result;
}

uint16_t scan_keypad_pressed(void) {
    uint16_t bounce, temp;
    bounce = scan_keypad();
    _delay_ms(15);
    temp = scan_keypad();
    bounce = bounce ^ temp;
    bounce = ~bounce;
    bounce = bounce & temp; /* keep only bits unchanged across both reads */
    return bounce;
}

uint16_t scan_keypad_rising_edge(void) {
    static uint16_t pressed = 0x00;
    uint16_t bounce = scan_keypad_pressed();
    uint16_t temp = pressed ^ bounce;
    temp = temp & bounce; /* keep only bits that went 0 -> 1 */
    pressed = bounce;
    return temp;
}

uint8_t keypad_to_ascii(void) {
    static const uint8_t codes[] = {'1', '2', '3', 'A', '4', '5', '6', 'B',
                                     '7', '8', '9', 'C', '*', '0', '#', 'D'};
    uint16_t keypad = scan_keypad_rising_edge();
    for (uint8_t counter = 0; counter < 16; counter++) {
        if (keypad & 1)
            return codes[counter];
        keypad >>= 1;
    }
    return 0;
}

/* ======================================================================
 * 2x16 LCD
 * ====================================================================== */

#define LCD_RS PD2
#define LCD_E  PD3

static void write_2_nibbles(uint8_t data) {
    uint8_t previous = PIND & 0x0F;
    uint8_t next = previous | (data & 0xF0); /* high nibble first */
    PORTD = next;
    PORTD |= (1 << LCD_E);
    PORTD &= ~(1 << LCD_E);

    next = previous | ((data << 4) & 0xF0); /* then low nibble */
    PORTD = next;
    PORTD |= (1 << LCD_E);
    PORTD &= ~(1 << LCD_E);
}

void lcd_data(uint8_t data) {
    PORTD |= (1 << LCD_RS);
    write_2_nibbles(data);
    _delay_us(250);
}

void lcd_command(uint8_t command) {
    PORTD &= ~(1 << LCD_RS);
    write_2_nibbles(command);
    /* The datasheet says ~39us is enough for most commands, but this
     * board's actual LCD hardware needs more than that in practice (also
     * noted in ex4-instructions.pdf) -- confirmed on hardware: 250us
     * wasn't enough specifically for "set DDRAM address" (e.g. jumping
     * to row 2, 0xC0), even though every other command used by lcd_init()
     * happened to work fine at 250us anyway. */
    _delay_ms(5);
}

void lcd_clear_display(void) {
    lcd_command(0x01);
    _delay_ms(5);
}

void lcd_init(void) {
    _delay_ms(200);

    /* Send "8-bit mode" 3x — the controller's own startup mode is
     * unknown, and this sequence reaches 8-bit mode from either. */
    for (uint8_t i = 0; i < 3; i++) {
        PORTD = (PORTD & 0x0F) | 0x30;
        PORTD |= (1 << LCD_E);
        PORTD &= ~(1 << LCD_E);
        _delay_us(250);
    }

    /* Now switch to 4-bit mode. */
    PORTD = (PORTD & 0x0F) | 0x20;
    PORTD |= (1 << LCD_E);
    PORTD &= ~(1 << LCD_E);
    _delay_us(250);

    lcd_command(0x28); /* 5x8 dots, 2 lines */
    lcd_command(0x0C); /* display on, cursor off */
    lcd_clear_display();
    lcd_command(0x06); /* increment address, no display shift */
}

void lcd_string(const unsigned char *str) {
    while (*str)
        lcd_data(*str++);
}

/* ======================================================================
 * DS18B20/DS1820, 1-Wire on PD4
 * ====================================================================== */

#define THERM_PORT PORTD
#define THERM_DDR  DDRD
#define THERM_PIN  PIND
#define THERM_DQ   PD4

#define THERM_INPUT_MODE()  (THERM_DDR &= (uint8_t) ~(1 << THERM_DQ))
#define THERM_OUTPUT_MODE() (THERM_DDR |= (1 << THERM_DQ))
#define THERM_LOW()         (THERM_PORT &= (uint8_t) ~(1 << THERM_DQ))
#define THERM_HIGH()        (THERM_PORT |= (1 << THERM_DQ))

#define THERM_CMD_CONVERTTEMP  0x44
#define THERM_CMD_SKIPROM      0xCC
#define THERM_CMD_RSCRATCHPAD  0xBE

uint8_t therm_reset(void) {
    uint8_t presence;
    THERM_OUTPUT_MODE();
    THERM_LOW();
    _delay_us(480);
    THERM_INPUT_MODE();
    THERM_LOW(); /* disable the internal pull-up */
    _delay_us(100);
    presence = THERM_PIN & (1 << THERM_DQ);
    _delay_us(380);
    return presence ? 0 : 1; /* device pulls the line low to answer */
}

void therm_write_bit(uint8_t bit) {
    THERM_OUTPUT_MODE();
    THERM_LOW();
    _delay_us(2);
    if (bit)
        THERM_HIGH();
    else
        THERM_LOW();
    _delay_us(58);
    THERM_INPUT_MODE();
    THERM_LOW();
    _delay_us(1);
}

uint8_t therm_read_bit(void) {
    uint8_t bit = 0;
    THERM_OUTPUT_MODE();
    THERM_LOW();
    _delay_us(2);
    THERM_INPUT_MODE();
    THERM_LOW();
    _delay_us(10);
    if (THERM_PIN & (1 << THERM_DQ))
        bit = 1;
    _delay_us(49);
    return bit;
}

void therm_write_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        therm_write_bit(byte & 1);
        byte >>= 1;
    }
}

uint8_t therm_read_byte(void) {
    uint8_t result = 0;
    for (uint8_t i = 0; i < 8; i++) {
        result >>= 1;
        if (therm_read_bit())
            result |= 0x80;
    }
    return result;
}

uint16_t therm_read_temperature(void) {
    uint8_t lo, hi;

    if (!therm_reset())
        return 0x8000;
    therm_write_byte(THERM_CMD_SKIPROM);
    therm_write_byte(THERM_CMD_CONVERTTEMP);
    while (!therm_read_bit())
        ; /* device pulls the line high once conversion completes */

    therm_reset();
    therm_write_byte(THERM_CMD_SKIPROM);
    therm_write_byte(THERM_CMD_RSCRATCHPAD);
    lo = therm_read_byte();
    hi = therm_read_byte();

    return ((uint16_t) hi << 8) | lo;
}

/* ======================================================================
 * ADC
 * ====================================================================== */

void adc_init(uint8_t channel) {
    /* REFS[1:0]=01 (AVCC ref), ADLAR=1 (left-adjusted) */
    ADMUX = 0b01100000 | (channel & 0x07);
    /* ADEN=1, ADSC=0, ADPS[2:0]=111 (/128 => ~125kHz @16MHz) */
    ADCSRA = 0b10000111;
}

void adc_set_channel(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
}

uint8_t adc_read8(void) {
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;
    return ADCH;
}

/* ======================================================================
 * Timer1 PWM on OC1A/PB1
 * ====================================================================== */

void pwm1_init(void) {
    TCCR1A = (1 << WGM10) | (1 << COM1A1); /* Fast PWM 8-bit, non-inverting */
    TCCR1B = (1 << WGM12) | (1 << CS10);   /* prescaler 1 */
    DDRB |= (1 << PB1);
}

void pwm1_set_duty(uint8_t duty) {
    OCR1AL = duty;
}

/* ======================================================================
 * USART0 + ESP8266 command helper
 * ====================================================================== */

void USART_Init(uint16_t ubrr) {
    UCSR0A = 0;
    UBRR0H = (uint8_t) (ubrr >> 8);
    UBRR0L = (uint8_t) ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (3 << UCSZ00); /* 8 data bits, 1 stop bit, no parity */
}

void usart_transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;
}

uint8_t usart_receive(void) {
    while (!(UCSR0A & (1 << RXC0)))
        ;
    return UDR0;
}

void usart_transmit_str(const unsigned char *str) {
    while (*str)
        usart_transmit(*str++);
    usart_transmit('\n');
}

unsigned char *usart_receive_str(unsigned char *buffer, uint8_t maxlen) {
    uint8_t i = 0;
    unsigned char c;
    do {
        c = usart_receive();
        if (i < maxlen - 1)
            buffer[i++] = c;
    } while (c != '\n');
    buffer[i > 0 ? i - 1 : 0] = '\0'; /* drop the trailing '\n' */
    return buffer;
}

unsigned char *usart_command(const unsigned char *message,
                              unsigned char *buffer, uint8_t maxlen) {
    usart_transmit_str(message);
    return usart_receive_str(buffer, maxlen);
}
