# Ex5 — I2C/TWI and the PCA9555 port expander

Source: `ex5-instructions.pdf`, 5th lab exercise (7/11/2025).

## What was given

This is where the **TWI/PCA9555 driver layer** that every later exercise
(6, 7, 8) reuses first shows up, handed out as complete, working code
(Παράδειγμα 5.1):

- `twi_init`, `twi_start`, `twi_start_wait`, `twi_write`, `twi_readAck`,
  `twi_readNak`, `twi_rep_start`, `twi_stop` — a full bit-banged-by-hardware
  TWI master built directly on the ATmega328PB's `TWCR0`/`TWSR0`/`TWDR0`
  registers, driving the 100kHz clock and handling the
  START/address/ACK/STOP handshake by polling `TWINT`.
- `PCA9555_0_write(reg, value)` / `PCA9555_0_read(reg)` — thin wrappers
  that turn the above into "write one register of the port expander" /
  "read one register", addressing it at `0x40` (`A0=A1=A2=0` on this
  board).
- The PCA9555's own register map (`REG_INPUT_0/1`, `REG_OUTPUT_0/1`,
  `REG_CONFIGURATION_0/1`, …) as a C `enum`.

Every `main.c` in this exercise (and in 6/7/8) starts from this exact
block of code, unmodified, and adds only the application logic below it.

**Note:** the assignment lists three deliverables (5.1, 5.2, 5.3); only
two sub-projects exist here (`5.1`, `5.2`) — 5.3 (an LCD "print your name"
exercise over the port expander) doesn't have a submitted implementation
in this repo.

## 5.1 — logic functions, output through the expander

Ζήτημα 5.1 asks for the same kind of boolean functions as Ex1's 1.2/1.3
(`F0 = (AB'+CBD)'`, `F1 = (A+C)(BD)` in the instructions' notation), this
time with A–D read live from `PORTB[3:0]` and F0/F1 driven out through
`IO0_0`/`IO0_1` of the PCA9555 instead of the microcontroller's own pins.

`main.c`'s `funct(value)` computes both functions with plain bitwise
AVR-C on the extracted bits, and `main()` polls `PINB & 0x0F`, and only
re-evaluates and re-writes (`PCA9555_0_write(REG_OUTPUT_0, ...)`) when the
button state actually changes from the previous pass (`if(val==prev)
continue;`) — avoiding pointless TWI traffic when nothing's pressed.

## 5.2 — a general port-expander I/O test

`main.c` here configures the expander's port 0 as output and port 1's
lower nibble as output / upper nibble as input
(`PCA9555_0_write(REG_CONFIGURATION_1, 0xF0)`) — the same split later used
for the keypad wiring in Ex6 — then just mirrors the (inverted) upper 4
input bits onto the lower 4 output bits in a loop. It reads more like a
warm-up check that the expander's second port and its input/output split
work correctly than a literal implementation of the "'1'/'2'/'3'/'A' key
lights PD0–PD3" task the instructions describe for 5.2 — no
`keypad_to_ascii`-style key decoding or `PORTD` LEDs appear here; that
piece of logic shows up fully built out in Ex6 instead.
