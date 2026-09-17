# Ex2 — External interrupts (INT0/INT1)

Source: `ex2-instructions.pdf`, 2nd lab exercise (17/10/2025).

## What was given

The instructions hand out two complete worked examples to build on:

- **Example 2.1** (assembly): a free-running 0–15 counter on `PORTB` with a
  500ms delay per step (the "counter program" flowchart) — the direct
  ancestor of every counter loop in this exercise.
- **Example 2.2** (C): the same idea plus both external interrupts wired up
  — `EICRA`/`EIMSK` configured for rising-edge INT0/INT1, ISRs on `PORTC`.
- The **debounce algorithm** (Σχήμα 2.4): inside an ISR, clear the
  interrupt's flag bit in `EIFR`, wait ~5ms, re-check the flag — if it's
  set again a bounce re-triggered the pin, so clear and re-check again
  until it stays clear.
- The register map for `EICRA` (edge-select bits `ISCn1:ISCn0`), `EIMSK`
  (enable bits), and `EIFR` (the flag bits `INTF1:INTF0`, clearable by
  writing them back with a `1`).

Every `.asm` file here reuses Example 2.1's `delay_mS` verbatim and follows
its counter-loop shape; the debounce sequence in both ISRs below is the
given algorithm applied directly (`ldi (1<<INTFn) / out EIFR / delay 5ms /
sbic EIFR,n / rjmp` back to re-check).

## 2.1 — INT1 counter with a freeze button

`main.asm` counts 0–15 on `PORTB` (Ζήτημα 2.1's base counter), and adds an
**INT1** handler (`ISR1`, vector `0x4`, falling edge via `ISC11:ISC10`)
that counts 0–31 and shows the result on `PORTC`. The debounce inside
`ISR1` is the given algorithm exactly.

One deviation worth knowing about: the assignment says the *freeze* button
is PD1; this code instead reads **PD6** (`sbic PIND,6`) to decide whether
to `cli`/keep interrupts enabled while looping — PD6 is one of the board's
other wired buttons, so this looks like a per-team pin substitution rather
than a mistake, but it means "PD1" in the assignment text maps to a
different physical pin than what actually gates the freeze here.

## 2.2 — INT0 counter + "how many buttons are pressed"

`main.asm` counts 0–31 on `PORTC` with a 1000ms step (Ζήτημα 2.2(A)). The
**INT0** handler (`ISR0`, vector `0x2`, falling edge) implements 2.2(B):
right after entry it snapshots `PINB` into `r10`, runs the same debounce
sequence, then walks 5 bits of that snapshot with `ror`/`brcc`, shifting a
`0b00011111` "thermometer" pattern in `r19` down by one for every pressed
(active-low) button bit — so `r19` ends up with as many trailing zero-bits
as buttons were held, lighting that many LEDs on `PORTC`. It checks 5
button bits (PB0–PB4) rather than literally "the 4 buttons PB4–PB1" the
text names, one bit wider than specified but the same mechanism.

## 2.3 — "light automatism" on PD3, in both languages

Ζήτημα 2.3 asks for a light-switch automatism: pressing PD3 (INT1) turns on
a "lamp" LED that should switch off automatically 4 seconds after the
*last* press, re-pressing while it's on should reset that 4s window, and
each press should also pulse the full `PORTB` LED bank for 1s.

`2.3_asm/main.asm` and `2.3_C/main.c` both implement a close variant of
this, triggered by the same INT1 setup as 2.1/2.2 (falling edge,
`EICRA`/`EIMSK`), inside the ISR itself rather than via a separate
main-loop timer:

1. Turn on all of `PORTB` for ~0.5s.
2. Drop to just `PB0` (the "lamp"), then call `sei()`/`sei` mid-ISR to
   re-enable interrupts *before* the ISR has returned.
3. Hold `PB0` for another couple of seconds (a `rep`/`Counter`-driven loop
   of ~51×50ms in the asm version; a `DELAY`-counted `while` of ~50×50ms in
   the C version), then turn everything off.

Because interrupts are re-enabled mid-ISR, a fresh PD3 press while it's
holding `PB0` fires `ISR1`/`ISR(INT1_vect)` again, re-entrantly — this is
what gives "refresh on re-press" behavior, via the AVR's own interrupt
nesting rather than an explicit software timer/flag. The concrete numbers
land close to but not exactly the spec (~0.5s pulse + ~2.5s hold ≈ 3s
total per press, vs. the assignment's 1s pulse / 4s hold), and both
`PORTB` LEDs (not a single dedicated "lamp" LED) light rather than just
PB3, but the reactive shape — press extends the on-time — is the same
idea, achieved more compactly than a counter-and-flag design would need.

## Naming note

Both `.asm` files output the raw counter/thermometer value straight to
`PORTC` rather than pre-shifting it onto bits `PC5:PC1` as the assignment
text says — so on hardware the same information lands one bit lower
(`PC4:PC0`) than the literal spec, without changing anything about the
logic itself.
