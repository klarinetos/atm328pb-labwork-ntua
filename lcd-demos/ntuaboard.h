/*
 * ntuaboard.h — shared driver library for the ntuAboard_G1 (ATmega328PB)
 *
 * Consolidates the helper functions handed out piecemeal across the
 * Exercise1-8 lab instructions (TWI, the PCA9555 I/O expander, the 4x4
 * keypad, the 2x16 LCD, the DS18B20 1-Wire sensor, USART/ESP8266, plus a
 * small ADC and Timer1-PWM layer that wasn't given as named functions
 * anywhere but follows the same register setup every exercise repeats by
 * hand) into one place, under the same names the exercises already use.
 *
 * See ../CLAUDE.md and each ExerciseN/walkthrough.md for where each part
 * of this actually came from and how it maps onto specific board pins.
 *
 * Usage: #include "ntuaboard.h" and compile ntuaboard.c alongside your
 * main.c (add it as a second source file in your Makefile/tasks.json —
 * it isn't picked up automatically by this repo's existing per-project
 * build files).
 *
 * Requires F_CPU defined before this header (16000000UL on this board).
 */

#ifndef NTUABOARD_H
#define NTUABOARD_H

#include <stdint.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Pulled in here, not just in ntuaboard.c, so anything that includes this
 * header gets register names (DDRx/PORTx/PINx/...) and _delay_ms/_delay_us
 * for free too -- every example in examples/ relies on that. */
#include <avr/io.h>
#include <util/delay.h>

/* ======================================================================
 * TWI (I2C) master — Ex5's given driver, built on TWI0 (PC4=SDA, PC5=SCL)
 * ====================================================================== */

#define SCL_CLOCK 100000UL /* twi clock in Hz */

void twi_init(void);

/* Issues a start condition and sends address+direction.
 * Returns 0 = device accessible, 1 = failed to access device. */
uint8_t twi_start(uint8_t address);

/* Same as twi_start, but retries (ack polling) until the device answers
 * instead of giving up — what every exercise actually calls. */
void twi_start_wait(uint8_t address);

/* Repeated start condition, address, transfer direction. Same return
 * convention as twi_start. */
uint8_t twi_rep_start(uint8_t address);

/* Terminates the transfer and releases the bus. */
void twi_stop(void);

/* Sends one byte to the previously-addressed device.
 * Returns 0 on success (ACK received), 1 on failure. */
uint8_t twi_write(uint8_t data);

/* Reads one byte, requesting another byte to follow (sends ACK). */
uint8_t twi_readAck(void);

/* Reads one byte, followed by a stop condition (sends NACK). */
uint8_t twi_readNak(void);

/* ======================================================================
 * PCA9555 — 16-bit I2C I/O expander (address 0x40 on this board: A0=A1=A2=0)
 * ====================================================================== */

typedef enum {
    REG_INPUT_0          = 0,
    REG_INPUT_1           = 1,
    REG_OUTPUT_0          = 2,
    REG_OUTPUT_1          = 3,
    REG_POLARITY_INV_0    = 4,
    REG_POLARITY_INV_1    = 5,
    REG_CONFIGURATION_0   = 6,
    REG_CONFIGURATION_1   = 7
} PCA9555_REGISTERS;

#define PCA9555_0_ADDRESS 0x40

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value);
uint8_t PCA9555_0_read(PCA9555_REGISTERS reg);

/* ======================================================================
 * 4x4 matrix keypad — wired to IO1 of the PCA9555 (rows IO1[3:0] out,
 * columns IO1[7:4] in, per Ex6). Requires
 * PCA9555_0_write(REG_CONFIGURATION_1, 0xF0) once at startup.
 * ====================================================================== */

/* Reads one row (1-4); returns which of its 4 columns are pressed, in
 * the low nibble. */
uint8_t scan_row(uint8_t row);

/* Scans all 4 rows; returns all 16 button states packed into one value
 * (bit n set = key n of codes[], see keypad_to_ascii, is held). */
uint16_t scan_keypad(void);

/* Debounced: keys currently held (level-detect — stays set the whole
 * time a key is down). */
uint16_t scan_keypad_pressed(void);

/* Debounced: keys newly pressed since the last call (edge-detect — set
 * for one call per press, regardless of how long it's held). */
uint16_t scan_keypad_rising_edge(void);

/* Decodes the (edge-detected) keypad state to the ASCII character of
 * whichever single key was just pressed ('0'-'9','A'-'D','*','#'), or 0
 * if none was. Assumes only one key is pressed at a time. */
uint8_t keypad_to_ascii(void);

/* ======================================================================
 * 2x16 character LCD — 4-bit interface on PORTD (PD2=RS, PD3=E,
 * PD[7:4]=data), per Ex4. Requires DDRD to have PD2/PD3/PD[7:4] as
 * outputs before use (lcd_init does not set DDRD itself, since PD[3:0]
 * may be shared with other uses on this board).
 * ====================================================================== */

void lcd_init(void);
void lcd_command(uint8_t command);
void lcd_data(uint8_t data);
void lcd_clear_display(void);

/* Writes a null-terminated string starting at the cursor. Does not clear
 * the display or move the cursor to a known position first — call
 * lcd_clear_display() (and a "set DDRAM address" lcd_command if you need
 * a specific line/column) yourself beforehand if you need that. */
void lcd_string(const unsigned char *str);

/* ======================================================================
 * DS18B20/DS1820 temperature sensor — 1-Wire on PD4, per Ex7.
 * ====================================================================== */

/* Returns 1 if a device responded to reset, 0 if the line is empty. */
uint8_t therm_reset(void);

void therm_write_bit(uint8_t bit);
uint8_t therm_read_bit(void);
void therm_write_byte(uint8_t byte);
uint8_t therm_read_byte(void);

/* Runs a full read: reset, Skip ROM, Convert T, wait for completion,
 * reset, Skip ROM, Read Scratchpad, both temperature bytes.
 * Returns the raw 16-bit two's-complement reading (0.0625 deg C/bit on
 * a DS18B20 in its default resolution, 0.5 deg C/bit on a DS1820), or
 * 0x8000 if no device answered the initial reset. */
uint16_t therm_read_temperature(void);

/* ======================================================================
 * ADC — single-ended, AVCC reference, left-adjusted (so an 8-bit-precision
 * reading is just ADCH, matching how every exercise that touches the ADC
 * actually uses it). Channel is one of the board's ADC0-ADC7 pins (see
 * docs/NtuaBoard_G1.pdf's PORTB/PORTC/PORTD connection tables for which
 * physical pin/pot/filter each channel maps to).
 * ====================================================================== */

/* Configures ADMUX/ADCSRA (prescaler 128 => ~125kHz ADC clock @16MHz) and
 * selects the given channel (0-7). Does not set any DDRx — the ADC's
 * digital input buffer is disabled by hardware automatically per pin
 * once that channel is selected and a conversion is run, so the pin
 * itself can stay whatever direction it already is. */
void adc_init(uint8_t channel);

/* Switches to a different channel without touching the rest of ADMUX. */
void adc_set_channel(uint8_t channel);

/* Triggers one conversion, blocks until it completes, returns the 8
 * most-significant bits of the result (ADCH). */
uint8_t adc_read8(void);

/* ======================================================================
 * Timer1 PWM — Fast PWM, 8-bit, non-inverting, on OC1A/PB1, per Ex3.
 * ====================================================================== */

/* Configures TCCR1A/TCCR1B (prescaler 1, so ~62.5kHz PWM @16MHz) and sets
 * PB1 as an output. Does not touch OCR1A — starts at whatever it already
 * held (0 after reset). */
void pwm1_init(void);

/* Sets the duty cycle, 0 (always low) - 255 (always high). */
void pwm1_set_duty(uint8_t duty);

/* ======================================================================
 * USART0 + ESP8266 command helper — 9600 baud, 8N1, per Ex8.
 * ====================================================================== */

#define MYUBRR ((F_CPU) / 16 / 9600UL - 1)

void USART_Init(uint16_t ubrr);
void usart_transmit(uint8_t data);
uint8_t usart_receive(void);

/* Sends a string followed by the required trailing '\n'. */
void usart_transmit_str(const unsigned char *str);

/* Reads characters into buffer (up to maxlen-1 of them) until a '\n' is
 * seen; null-terminates in place of it. Returns buffer, so this can be
 * used inline (e.g. lcd_string(usart_receive_str(buf, sizeof buf))).
 *
 * Note: earlier per-exercise versions of this function (Ex8) took no
 * arguments and returned a pointer to a local stack array, which is
 * undefined behavior the moment the function returns — this version
 * fixes that by having the caller supply the buffer. */
unsigned char *usart_receive_str(unsigned char *buffer, uint8_t maxlen);

/* Sends an ESP8266 command (see docs — "ESP:connect", "ESP:url:\"...\"",
 * "ESP:payload:[...]", "ESP:transmit", ...) and reads back its one-line
 * reply into buffer. Returns buffer. */
unsigned char *usart_command(const unsigned char *message,
                              unsigned char *buffer, uint8_t maxlen);

#endif /* NTUABOARD_H */
