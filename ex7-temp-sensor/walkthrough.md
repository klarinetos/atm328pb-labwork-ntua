# Ex7 — DS1820/DS18B20 temperature sensor, 1-Wire

Source: `ex7-instructions.pdf`, 7th lab exercise (21/11/2025).

## What was given

The instructions hand out a complete **assembly** 1-Wire bit-banging
layer for the sensor on `PD4`, explained routine by routine:

- `one_wire_reset` — 480µs low pulse, then samples `PD4` 100µs later to
  detect a device's presence pulse.
- `one_wire_receive_bit` / `one_wire_transmit_bit` — one read/write time
  slot each, with the exact µs timings the DS18B20 datasheet requires.
- `one_wire_receive_byte` / `one_wire_transmit_byte` — 8 calls to the bit
  routines, LSB first.

Ζήτημα 7.1 then asks for these **ported to C under the same names**
("the C functions must correspond to the given routines"), assembled into
one `temperature()` routine that: resets and checks for a device, sends
**Skip ROM** (`0xCC`, since only one sensor is on the bus), sends
**Convert T** (`0x44`), polls a read bit until it comes back `1`
(conversion done), resets again, Skip ROM again, sends **Read Scratchpad**
(`0xBE`), reads the 2 temperature bytes, and returns `0x8000` if no device
answered the initial reset.

## 7.1 — the ported routines, with the assembly's own routine left unfinished

`main.c` ports every one of the given assembly routines to C, faithfully,
under their exact given names: `one_wire_reset`, `one_wire_receive_bit`,
`one_wire_transmit_bit`, `one_wire_receive_byte`, `one_wire_transmit_byte`
are all complete and correct translations, timing constants included.

What's **not** finished is the routine Ζήτημα 7.1 actually asks you to
build on top of them:

```c
uint16_t temperature(){
    
    
}
```

— an empty stub. There's no `main()` either, so this file can't be
flashed (see the note in the repo root's `CLAUDE.md`); as a standalone
piece it's the faithful port of the given library, with the actual
"reset → Skip ROM → Convert T → wait → reset → Skip ROM → Read Scratchpad"
sequence from Ζήτημα 7.1 never written.

## 7.2 — the same protocol, finished, under different names

`main.c` here doesn't reuse 7.1's ported functions or names at all — it's
an independent implementation of the identical protocol via `THERM_*`
macros for the pin (`THERM_OUTPUT_MODE()`, `THERM_LOW()`, …) and functions
named `therm_reset`, `therm_read_bit`/`therm_write_bit`,
`therm_read_byte`/`therm_write_byte`. Crucially, **`therm_read_temperature()`
is exactly the routine 7.1 left as a stub**: reset-and-check, `0xCC` Skip
ROM, `0x44` Convert T, poll for completion, reset, `0xCC` again, `0xBE`
Read Scratchpad, read both bytes into a 16-bit result, `0x8000` if no
device — matching Ζήτημα 7.1's spec exactly, just finished here instead of
in 7.1.

Where this diverges from Ζήτημα **7.2** specifically (LCD display, 3-digit
signed decimal, a "NO Device" message) is the `main()`: it doesn't touch
the LCD at all. It takes `temp_integer()` — the temperature's integer part
(bits 4–9 of the reading, i.e. `>>4 & 0x3F`) — and writes it straight to
`PORTB` in binary once a millisecond, in a tight loop. It's a real,
flashable, working temperature reading (unlike 7.1), just displayed as raw
binary on LEDs rather than the LCD readout the assignment describes.
