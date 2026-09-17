#define F_CPU 16000000UL
#include "../ntuaboard.h"

/* Ex5.2: mirror the PCA9555 IO1 upper nibble (inputs) onto its IO0 lower
 * nibble (outputs) -- an I/O-expander smoke test. See
 * ../../ex5-i2c/walkthrough.md. */

int main(void) {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    PCA9555_0_write(REG_OUTPUT_1, 0x00);

    while (1) {
        uint8_t val = PCA9555_0_read(REG_INPUT_1) >> 4;
        PCA9555_0_write(REG_OUTPUT_0, (uint8_t) ~val);
    }
}
