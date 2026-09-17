# lib/examples

The lab exercises that actually touch a peripheral `../ntuaboard.h`
covers, rewritten to use it instead of each carrying its own copy of the
TWI/PCA9555/LCD/DS18B20/USART driver code. Same application behavior as
the original submission in each case (see the corresponding
`ExN-*/walkthrough.md` for exactly what that behavior is and how it maps
to the assignment) unless noted in the file's own header comment.

| File | From | Notes |
|---|---|---|
| `3.2.c` | `ex3-adc-pwm/3.2` | 8-band ADC bargraph rewritten as one shift instead of an if/else chain |
| `3.3.c` | `ex3-adc-pwm/3.3` | |
| `4.2.c` | `ex4-adc-lcd/4.2` | 2-decimal voltage via integer math instead of the original's manual search loop |
| `4.3.c` | `ex4-adc-lcd/4.3` | |
| `5.1.c` | `ex5-i2c/5.1` | |
| `5.2.c` | `ex5-i2c/5.2` | |
| `6.1.c` | `ex6-keypad/6.1` | |
| `6.2.c` | `ex6-keypad/6.2` | |
| `6.3.c` | `ex6-keypad/6.3` | timings nudged to match the assignment's 3s/6s exactly |
| `7.2.c` | `ex7-temp-sensor/7.2` | now actually shows on the **LCD** (signed, `NO Device` handled) — the original only ever wrote the raw reading to `PORTB` in binary |
| `8.1.c` | `ex8-final-project/8.1` | now shows literal `Success`/`Fail`, not the raw ESP reply, on every step |
| `8.2.c` | `ex8-final-project/8.2` | |
| `8.3.c` | `ex8-final-project/8.3` | **NURSE CALL is now actually wired to the keypad** — in the original, that status and `keypad_to_ascii()` both existed but nothing called the latter, so the status was dead code (see the walkthrough) |

**Not included:** `ex2-interrupts/2.3_C` and every `.asm` exercise. `2.3_C`
is pure GPIO/interrupt handling — it doesn't touch TWI, the LCD, the ADC,
PWM, or the sensor, so there's nothing in `ntuaboard.h` for it to use.
The `.asm` exercises are a different language entirely; the library is
C-only.

`ex7-temp-sensor/7.1` isn't here as its own file either — the routine it
leaves as a stub is exactly `therm_read_temperature()`, so "7.1 finished"
*is* `7.2.c`.

## Building

```
make EX=6.3        # build out/6.3.hex
make EX=6.3 load    # build, then flash to the board
make list            # see all the example names
make EX=6.3 clean   # remove out/6.3.{elf,hex}
```

Every example was compiled and linked against `../ntuaboard.c` with
`-Wall -Wextra` before being committed — zero warnings, all 13.
