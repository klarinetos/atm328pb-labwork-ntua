# Ex3 — Timer1 PWM and the ADC

Source: `ex3-instructions.pdf`, 3rd lab exercise (24/10/2025).

## What was given

- **Timer1 register theory**: `TCCR1A`/`TCCR1B`'s `WGMn` bits selecting
  Fast PWM 8-bit mode, `COM1A1:COM1A0` selecting non-inverting output on
  `OC1A`/`PB1`, `CS1n` selecting the prescaler, and the PWM frequency
  formula `f = fclk / (N·(1+TOP))`.
- **Example 3.1** (C): the exact skeleton every sub-exercise here builds
  on — `TCCR1A = (1<<WGM10)|(1<<COM1A1)`, `TCCR1B = (1<<WGM12)|(1<<CS11)`,
  a `duty` variable swept 0→255→0 through `OCR1AL` to fade an LED on PB1.
- **ADC register theory**: `ADMUX` (reference/channel select, `ADLAR` for
  left-adjusted results), `ADCSRA` (`ADEN`/`ADSC`/prescaler bits), and
  **Example 3.2** (assembly): a free-running ADC0 read loop, left-adjusted,
  result's top byte (`ADCH`) mirrored straight onto `PORTD`.
- The board's own analog-voltage sources: 4 potentiometers and 3 PWM-driven
  RC filters, jumper-selectable onto the ADC's first 4 channels.

## 3.1 — button-stepped duty cycle, from a lookup table

Ζήτημα 3.1 asks for Timer1 in Fast PWM 8-bit on PB1 at a specific
frequency, with duty cycle stepped by 6% per button press (PB4 up, PB5
down) between 2% and 98%, using **pre-computed `OCR1A` values from a
table** rather than computing them at runtime.

`main.asm`'s `table:` holds exactly that: `5,25,46,66,87,107,127,...,251`
— 13 entries, each ≈6% of 255 apart, `127` (the middle one, ≈50%) loaded
as the initial duty cycle. `Z` (`ZH:ZL`) points into the table; `PD1_pressed`/
`PD2_pressed` (read via `sbis PIND,1`/`sbis PIND,2` in the `main` loop)
`adiw`/`sbiw` the pointer by one table entry and re-`lpm` the new
`OCR1AL`, clamped by a counter (`r17`, 0–12) so it can't walk off either
end of the table. One thing to know: the code sets `CS11` (prescaler 8),
giving a PWM frequency of ~7.8kHz rather than the 62.5kHz the instructions
name (which needs prescaler 1, `CS10`) — the table-driven stepping
mechanism itself matches the assignment precisely, just at an eighth of
the specified frequency.

## 3.2 — PWM + ADC bargraph, with duty buttons layered in

Ζήτημα 3.2 asks for the same PWM output plus an ADC reading (averaged over
16 samples) driving one of 5 LEDs by voltage band.

`main.c` reuses Example 3.1's Timer1 setup and 3.1's duty-cycle table,
still stepped by the same PB4/PB5 buttons, and adds a single (not
16-sample-averaged) `ADCH` read each pass of the loop, classified into
**8** exclusive bands (`y<32`, `<64`, … `<224`, else) each lighting a
*different single bit* of `PORTD` (`0x01`…`0x80`) — a one-LED-per-band
bargraph rather than the assignment's cumulative "0–4 LEDs on" scheme, and
without the specified 16-sample averaging. The core idea — PWM plus a
live ADC readout on LEDs — is there; the exact averaging and LED-count
details differ from the letter of 3.2.

## 3.3 — two brightness-control modes

Ζήτημα 3.3 asks for the same PWM output controllable two ways: **mode 1**
by the PB4/PB5 buttons (as in 3.1), **mode 2** by potentiometer POT1
directly, switched by two mode-select buttons.

`main.c` implements exactly this split — `mode==1` re-runs 3.1's
table-stepped duty cycle (buttons read on `PD1`/`PD2` here), `mode==2`
instead reads `ADCH` from channel 0 (`ADMUX=0b01100000`, `CS10` this time,
so full-speed conversions) and assigns it to `OCR1AL` directly, no table —
the potentiometer's position becomes the duty cycle 1:1. Mode selection
reads `PD6`/`PD7` rather than the `PD0`/`PD1` the assignment names for
that role, in keeping with the same kind of per-team pin substitution seen
elsewhere in these exercises.
