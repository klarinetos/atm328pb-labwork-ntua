#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex5.1: F0/F1 of PORTB[3:0] (A,B,C,D), output through the PCA9555's
 * IO0_0/IO0_1. See ../../ex5-i2c/walkthrough.md. */

static uint8_t funct(uint8_t value) {
    uint8_t A = value & 0x01;
    uint8_t B = (value >> 1) & 0x01;
    uint8_t C = (value >> 2) & 0x01;
    uint8_t D = (value >> 3) & 0x01;

    uint8_t f1 = (uint8_t)(A & C & (B | D));
    uint8_t f0 = (uint8_t) !(((!A) & B) | ((!B) & C & D));

    return (uint8_t)((f1 << 1) | f0);
}

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00); /* IO0 as output */
    DDRB = 0x00;                                /* PORTB as input */

    uint8_t prev = 0xFF;
    while (1) {
        uint8_t val = PINB & 0x0F;
        if (val != prev) {
            prev = val;
            PCA9555_0_write(REG_OUTPUT_0, funct(val));
        }
    }
}
