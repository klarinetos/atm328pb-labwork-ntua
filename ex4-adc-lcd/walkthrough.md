# Ex4 — 2×16 character LCD + ADC

Source: `ex4-instructions.pdf`, 4th lab exercise (31/10/2025).

## What was given

The LCD on ntuAboard_G1 is driven 4-bit, no busy-flag readback (fixed
delays instead), through `PD2` (RS) and `PD3` (E) plus the upper nibble of
`PORTD` for data — and the instructions hand out a **complete driver**,
routine by routine, that every project in this exercise is built directly
on:

- `write_2_nibbles` — sends a byte's two nibbles out over `PORTD[7:4]`,
  toggling `PD3` (Enable) between them, leaving `PD2`/`PD3` themselves
  untouched so the same routine serves both data and command writes.
- `lcd_data` / `lcd_command` — set `PD2` (RS) then call `write_2_nibbles`,
  followed by a 250µs settle delay.
- `lcd_clear_display`, `lcd_init` — the fixed power-on sequence (3× the
  "switch to 8-bit" command since the controller's own mode on cold start
  is unknown, then "switch to 4-bit", then function-set/display-on/clear).

Every `.asm`/`.c` file here reimplements these same named routines (ported
line-for-line into C for the two C exercises) rather than inventing its
own LCD layer.

## 4.1 — ADC-interrupt-driven voltage readout, in assembly

Ζήτημα 4.1 asks for an interrupt-driven ADC read, printing the resulting
voltage on the LCD to two decimal places.

`main.asm` sets `ADMUX=0b01100010` (AVCC reference, left-adjusted,
**ADC2**) and enables the ADC conversion-complete interrupt (vector
`0x2A`, `ADCint`), then starts one conversion and idles in `sei`/`rjmp
main`. `ADCint` takes `ADCH` (the top 8 bits of the 10-bit result, thanks
to left-adjustment) and computes `ADCH×5` as one 8×8 `mul` — the high
result byte (`r1`) is the integer-volts digit, printed directly; the low
byte (`r0`), shifted right 5 (÷32), selects one of 8 branches
(`zero`…`seven`) that each print a fixed two-character string (`"00"`,
`"12"`, `"25"`, `"37"`, `"50"`, `"62"`, `"75"`, `"87"`) — a division-free
way to get two fractional digits out of a single multiply instead of
running an actual divide routine. `y=255` (input pegged high) is
special-cased to print `4,99`. Note the instructions ask for POT4/**ADC3**
specifically; this reads ADC2 instead — the same per-team pin variation
seen elsewhere in these exercises.

## 4.2 — the same readout, polled instead of interrupt-driven, in C

Ζήτημα 4.2 asks for the same voltage display, but by **polling** `ADSC`
in a loop rather than using the interrupt.

`main.c` does exactly that (`ADCSRA |= (1<<ADSC); while(ADCSRA &
0b01000000);`), then computes `y = ADCH*5.0/256` as a `double` and extracts
two decimal digits without ever calling `printf` on a float (avr-libc's
float `printf` isn't linked in by default, and wasn't asked for): it scans
`i` from 0 up until `y < 0.01*(i+1)`, then reads the 3 decimal digits back
out of `i` via `/100`, `%100/10`, `%10` — a manual "search for how many
hundredths" instead of a float-to-string routine.

## 4.3 — CO gas sensor simulation

Ζήτημα 4.3 asks for a simulated CO sensor (via POT4) driving a level
bargraph on 6 LEDs, with a "GAS DETECTED"/"CLEAR" LCD message and blinking
LEDs once the level crosses 75ppm.

`main.c` reads `ADCH` from the ADC each pass; below `~51` (≈1.0V of the
0–5V pot range, standing in for the 75ppm threshold) it's in normal mode,
lighting a growing subset of `PORTB`'s 6 LEDs (`0b000111` → `0b001111` →
`0b011111` → `0b111111`) by voltage band, steady. Crossing `51` flips
`mode` to the alarm state: the same LED bank now blinks at 300ms on/off,
and `lcd_string("GAS DETECTED")` is written once on the transition
(`change==1`, so it isn't rewritten every loop). Dropping back below the
threshold flips `mode` back and writes `"CLEAR"` once the same way.
