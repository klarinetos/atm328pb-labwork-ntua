# Understanding every program in `paliathemata`

This guide explains the **current C implementations**, including the earlier
`epan25.c` and all 14 programs adapted from the theory PDF. Where the PDF's
comments disagree with its code, the distinction is stated explicitly.
Expected results below are derived from the source, not a claim that every
program has been tested on hardware.

## Choose the program you actually want to run

Each `.c` has its own `main()` and is a separate firmware image. You do not
run all the files together. Loading one replaces the previous program.
**The file open in the editor does not select the Makefile target.**

In a terminal inside `lib/paliathemata`:

```powershell
make list
make EX=pb_logic_function_lcd
make EX=pb_logic_function_lcd load
```

The first command lists choices, the second builds, and the third builds and
programs/verifies the board. `EX` is the filename without `.c`.
The output here is `out/pb_logic_function_lcd.hex`.
Plain `make` or `make load` selects **`full`, meaning `epan25.c`**, regardless
of which editor tab is active. From the repository root, add
`-C lib/paliathemata` to these commands.

| Program / section | What you observe |
|---|---|
| [1. Button position](#1-button-position) | LCD shows which PB0–PB3 button is pressed |
| [2. One logic function](#2-one-logic-function) | LCD shows a Boolean result, 0 or 1 |
| [3. Count pressed buttons](#3-count-pressed-buttons) | LCD shows how many PB0–PB5 buttons are pressed |
| [4. Name](#4-name) | LCD shows NIKOS starting at column 3 |
| [5. Two logic functions](#5-two-logic-functions) | Separate PB and PC functions on the two LCD lines |
| [6. Train](#6-train) | An O travels across the LCD in response to buttons |
| [7. Fixed PWM](#7-fixed-pwm) | PB1 produces approximately 20% duty PWM |
| [8. Keypad and ASCII](#8-keypad-and-ascii) | ASCII bit count on LEDs; # lights PC0 |
| [9. PWM ramps](#9-pwm-ramps) | PB1 duty rises/falls or rises/resets |
| [10. INT1 and ADC](#10-int1-and-adc) | Button release triggers a 1600 mV comparison |
| [11. Six-bit input, 2024](#11-six-bit-input-2024) | Binary input and bit-count result every 3.5 s |
| [12. Logic, shift 1/2025](#12-logic-shift-12025) | Four inputs and two results every 5 s |
| [13. Logic, shift 2/2025](#13-logic-shift-22025) | Different pins/functions, every 3 s |
| [14. INT0 and ADC, 2025](#14-int0-and-adc-2025) | Button release triggers a 1450 mV comparison |
| [15. Combined exam](#15-combined-exam) | Periodic digital inputs plus last two keypad characters |

## Basics used throughout the code

### Input pins, pull-ups, and active-low buttons

`DDRB` controls direction: bit 0 means input, bit 1 means output.
`PINB` reads electrical levels. `PORTB` sets output levels for outputs and
enables pull-ups for inputs.

For example, `exam_pb_inputs(0x0F)` configures PB0–PB3 as inputs with pull-ups.
Each button connects its pin to GND when pressed:

| Button state | Electrical level read from PINB | After `~PINB` and masking |
|---|---|---|
| Released | 1 | 0: not pressed |
| Pressed | 0 | 1: pressed |

Most button exercises invert the input so that **logical 1 means pressed**.
The 2024 six-bit exercise and the input part of `epan25.c` do **not** invert:
their displayed 1 means a physically high pin, usually a released button.

Masks choose which pins matter. `0x0F = 00001111` selects PB0–PB3;
`0x3F = 00111111` selects PB0–PB5; `0x1E = 00011110` selects PB1–PB4.
`(value >> n) & 1` extracts bit n as the number 0 or 1.

### LCD wiring, characters, and positions

All LCD programs here use the existing `ntuaboard` PORTD driver:

| MCU pin | LCD signal |
|---|---|
| PD2 | RS |
| PD3 | E |
| PD4–PD7 | LCD D4–D7 |

Use the lab's matching 4-bit LCD wiring, with R/W held in write mode and
the LCD power/contrast connections in place. The PDF's expander LCD setup
is different: connecting only an LCD through PCA9555 will not work with
these PORTD LCD calls.

`lcd_data('1')` displays the character 1. `lcd_data(1)` sends character code
1 instead. Thus `'0' + result` converts a single numeric digit to its ASCII
character. It is not a general conversion for numbers greater than 9.

For visible columns numbered 1–16:

```text
First-line address  = 0x80 + (column - 1)
Second-line address = 0xC0 + (column - 1)
```

Examples: `0x82` = line 1, column 3; `0xC4` = line 2, column 5.
In display illustrations below, `·` means a blank cell; no dot is actually printed.

Two different bit-printing orders occur in these exercises:

- `exam_binary8(value)` prints **bit7 first**, the usual binary number notation.
- `exam_bits_low_first(value, 4)` prints **bit0 first**, useful for showing
  A, B, C, D in pin order. `1000` in this display can mean A=1, not the number 8.

### What the helper files do

`exam_helpers.h` includes `../ntuaboard.h`; this makes the existing library
declarations, AVR registers and delay functions available to the exercises.
The Makefile also compiles `../ntuaboard.c`, which supplies the peripheral drivers.
Including a header alone does not link those implementations.

| Helper | Meaning |
|---|---|
| `exam_lcd_init()` | Set PD2–PD7 to outputs, initialize and clear LCD |
| `exam_pb_inputs(mask)` | Configure selected PORTB pins as inputs with pull-ups |
| `exam_popcount(value)` | Count 1 bits, not their positions or numeric value |
| `exam_binary8(value)` | Print exactly eight bits, MSB first |
| `exam_bits_low_first(value, count)` | Print selected number of bits, LSB first |
| `exam_adc10_init()` | Use ADC1/PC1 and AVCC; set right-adjusted 10-bit results |
| `exam_adc10_read()` | Start conversion, wait until complete, return 0–1023 |
| `keypad_to_ascii()` | Library helper: return a newly pressed character, or 0 |
| `pwm1_init()` / `pwm1_set_duty()` | Library helpers: initialize PB1 PWM / set compare value |

`exam_adc10_read()` exists because `ntuaboard`'s `adc_read8()` only returns
eight bits. The ADC threshold exercises retain the source's ten-bit precision.
`epan25.c` directly includes `ntuaboard.h` and does not use `exam_helpers.h`.

All timing assumes an actual **16 MHz CPU clock**. Defining `F_CPU` tells
the compiler the clock frequency; it does not configure the physical clock.
LED descriptions assume active-high LED circuits connected to the named pins.

## 1. Button position

Source: [pb0_pb3_button_position_lcd.c](pb0_pb3_button_position_lcd.c). PDF pp. 8–9.

**Task:** identify the lowest-numbered pressed button among PB0–PB3 and
display its position as 1–4. Display 0 if none is pressed.

The program enables pull-ups, initializes the LCD, then repeatedly reads
`~PINB & 0x0F`. When the result is nonzero, it starts `position` at 1 and
shifts the result right until its lowest bit is 1. Each shift increments
the position. The answer is written at line 1, column 1.

Example: only PB2 is pressed. The masked pressed-state byte is `00000100`.
After two right shifts it becomes `00000001`, and `position` becomes 3.

| Pressed buttons | LCD |
|---|---|
| None | `0` |
| PB0 | `1` |
| PB1 | `2` |
| PB2 | `3` |
| PB3 | `4` |
| PB1 and PB3 | `2` |

It checks the current state about every 20 ms plus LCD overhead. It does not
remember an earlier press, count presses, or list all held buttons. The delay
limits refresh rate; it is not a full debounce algorithm.

Build/load: `make EX=pb0_pb3_button_position_lcd load`.

## 2. One logic function

Source: [pb_logic_function_lcd.c](pb_logic_function_lcd.c). PDF pp. 9–10.

**Task:** compute `F = NOT((A AND NOT B) OR (NOT B AND D))` and show 0 or 1
at line 1, column 1. A=PB0, B=PB1, D=PB3, with **pressed = 1**.
PB2 corresponds to C in the four-input naming scheme but C is unused.

The input is inverted. The expressions `in & 1`, `(in >> 1) & 1` and
`(in >> 3) & 1` extract A, B and D. C's `!`, `&&`, and `||` then implement
NOT, AND and OR. The result becomes a digit through `'0' + result`.

Factoring NOT B and applying De Morgan's law gives a useful interpretation:

```text
F = B OR (NOT A AND NOT D)
```

So B pressed always makes F=1. With B released, F=1 only if both A and D
are also released. PB2 never changes the result.

| A | B | D | LCD F |
|---|---|---|---|
| 0 | 0 | 0 | 1 |
| 0 | 0 | 1 | 0 |
| 0 | 1 | 0 | 1 |
| 0 | 1 | 1 | 1 |
| 1 | 0 | 0 | 0 |
| 1 | 0 | 1 | 0 |
| 1 | 1 | 0 | 1 |
| 1 | 1 | 1 | 1 |

Test: release everything → `1`; press PB0 → `0`; keep PB0 pressed and also
press PB1 → `1`. The LCD updates about every 20 ms plus write overhead.

Build/load: `make EX=pb_logic_function_lcd load`.

## 3. Count pressed buttons

Source: [pb0_pb5_popcount_lcd.c](pb0_pb5_popcount_lcd.c). PDF pp. 10–11.

**Task:** count how many of PB0–PB5 are currently pressed, and display 0–6.

`~PINB & 0x3F` converts the six active-low button states into six logical
bits. `exam_popcount()` repeatedly adds the lowest bit to a counter and
shifts right. The total becomes one ASCII digit at line 1, column 1.

For PB0, PB2 and PB5 pressed, the byte is `00100101`. There are three 1 bits,
so the LCD displays `3`, even though that byte's numeric value is 37.

Release all buttons → `0`; hold any one → `1`; hold all six → `6`.
The program samples roughly every 20 ms. It counts held buttons, not the
number of press events over time. Unlike exercise 1, it does not give the
position of the first button.

The PDF's `count-1` was corrected to the actual count requested by its title.

Build/load: `make EX=pb0_pb5_popcount_lcd load`.

## 4. Name

Source: [name_lcd_third_column.c](name_lcd_third_column.c). PDF pp. 12–13.

**Task:** show a name starting at the third position of the LCD's first line.

After LCD initialization, `lcd_command(0x82)` moves to column 3 and
`lcd_string()` prints `NIKOS`. The program then stays in an empty loop.
The LCD retains its characters, so repeatedly writing them is unnecessary.

```text
Line 1: ··NIKOS··········
Line 2: ················
```

Change the string literal to change the name. There are 14 visible positions
from column 3 through column 16. Ordinary ASCII text matches this example;
UTF-8 Greek text cannot simply be sent as if each byte were a Greek LCD glyph.

The PDF advanced three positions from column 1, which would reach column 4.
This code uses the third column specified by its title.

Build/load: `make EX=name_lcd_third_column load`.

## 5. Two logic functions

Source: [pb_pc_two_logic_functions_lcd.c](pb_pc_two_logic_functions_lcd.c). PDF pp. 13–14.

**Task:** read two independent sets of active-low buttons and show one Boolean
answer on each LCD line, both at column 1.

| Line | Input names | Implemented expression |
|---|---|---|
| 1 | A=PB0, B=PB1, C=PB2, D=PB3 | `!((A && !B) || (!B && D))` |
| 2 | A1=PC0, B1=PC1, C1=PC2, D1=PC3 | `!(!A1 || B1 || C1 || D1)` |

Line 1 is exactly exercise 2. Line 2 simplifies to
`A1 && !B1 && !C1 && !D1`: it is 1 only when **PC0 alone** is pressed in
that set. Both bytes are read, inverted, split into bits, evaluated and
displayed before the next 20 ms delay.

Test with all buttons released: first line `1`, second line `0`.
Press PC0 only: second line becomes `1`; adding PC1 makes it `0`.
Changing PORTC buttons does not change the PORTB function.

**Source ambiguity:** the PDF's comment suggests a different second formula,
`(!A1 || B1) && (C1 || D1)`. This program preserves the PDF's executable
expression, not that conflicting comment. Check the original exam statement
before treating either as the required formula.

Build/load: `make EX=pb_pc_two_logic_functions_lcd load`.

## 6. Train

Source: [lcd_train_two_buttons.c](lcd_train_two_buttons.c). PDF pp. 14–15.

**Task:** move the character `O` along the first LCD line using two buttons.
PB0 requests left→right, PB1 right→left. Both buttons are active-low.

At startup `column=0`, so O appears at visible column 1. At the left endpoint
only the PB0 button pattern is accepted. The code waits 20 ms and checks it
again. It then makes 15 moves, each after a 1 s delay: erase the old cell,
increment the position, and write O at the new cell. It finishes at column 16.
At that endpoint PB1 starts the corresponding return trip.

```text
Startup: O···············
1st step: ·O··············
...
15th step: ···············O
```

Each trip takes approximately 15 s plus LCD overhead. It is not a scrolling
word and does not shift the entire display.

Buttons are not scanned while moving, so you cannot reverse halfway through.
Pressing both buttons fails the exact-pattern check. This is endpoint level
detection: if the valid return button is already held when the train arrives,
the return trip can begin without a new press.

Build/load: `make EX=lcd_train_two_buttons load`.

## 7. Fixed PWM

Source: [pb1_pwm_percentage.c](pb1_pwm_percentage.c). PDF pp. 15–16.

**Task:** generate a PWM signal on PB1/OC1A with a selectable duty percentage.
There is no LCD and no button input. `DUTY_PERCENT` defaults to 20.

`pwm1_init()` sets up Timer1 and PB1. The code then overrides its prescaler
to /8, matching the PDF. The compare value is calculated as
`255 * DUTY_PERCENT / 100` and passed to `pwm1_set_duty()`.

For 20%, the compare value is 51. At 16 MHz, 8-bit Fast PWM with /8 has
frequency `16000000 / (8 * 256) = 7812.5 Hz`, or a 128 µs period.
The high-time fraction is approximately the selected percentage: register
quantization and Fast PWM's compare behavior mean it is not an exact real-valued percentage.

An attached active-high LED appears dimmer than at 100%, rather than visibly
blinking thousands of times a second. A scope or logic analyzer shows the
pulse train. The empty main loop does not stop PWM: the timer runs in hardware.

Edit `DUTY_PERCENT`, rebuild, and load to change the duty. Values outside
0–100 trigger a compile error. At 0% and 100% the code disconnects the timer
output and drives PB1 steadily low/high, respectively.

Build/load: `make EX=pb1_pwm_percentage load`.

## 8. Keypad and ASCII

Source: [keypad_ascii_popcount_leds.c](keypad_ascii_popcount_leds.c). PDF pp. 16–17.

**Task:** for each newly pressed keypad key, count the 1 bits in its ASCII
code, subtract an offset, and place the numeric result on PB0–PB5.
For `#`, also light PC0 for 1 s.

The matrix keypad is connected through PCA9555: IO1[3:0] are row outputs,
IO1[7:4] are column inputs, and PC4/PC5 carry TWI. The program initializes
that interface and calls `keypad_to_ascii()` continuously. A returned 0
means no new key; otherwise the returned byte is a character code.

For example, the key `5` gives ASCII `0x35 = 00110101`, **not numeric 5**.
There are four 1 bits in that ASCII byte. With the default offset 2, the
output value is 2, so PB1 is high and the other PB0–PB5 outputs are low.
This is a binary number on LEDs, not a bar of two illuminated LEDs.

| Key | ASCII bits | Popcount | Default result | High result pins |
|---|---|---|---|---|
| `0` | `00110000` | 2 | 0 | None |
| `1` | `00110001` | 3 | 1 | PB0 |
| `5` | `00110101` | 4 | 2 | PB1 |
| `7` | `00110111` | 5 | 3 | PB0, PB1 |
| `#` | `00100011` | 3 | 1 | PB0, after the PC0 pulse |

For #, PC0 goes high, the code waits 1 s, PC0 goes low, and only then is
the PB result updated. During that second the previous PB result remains
and new short key presses can be missed. Otherwise the result stays until
the next recognized key. Hold does not repeatedly generate presses; release
and press again to repeat the same key. Use one key at a time.

**Source ambiguity:** the PDF subtracts 1 twice. `ASCII_COUNT_OFFSET=2`
preserves that behavior explicitly. Set it to 0 for the plain bit count or
1 for a single subtraction if the actual statement requires that. The code
clamps results below zero to zero.

Build/load: `make EX=keypad_ascii_popcount_leds load`.

## 9. PWM ramps

Source: [pwm_two_button_ramp_modes.c](pwm_two_button_ramp_modes.c). PDF pp. 17–19.

**Task:** vary PB1's PWM duty automatically, selecting between two patterns
with PC1 and PC0 buttons. There is no LCD or ADC measurement.

| Selection | Duty sequence | Interval between changes |
|---|---|---|
| Startup or PC1 pressed | 10, 20, …, 90, 80, …, 10, 20, … | About 500 ms |
| PC0 pressed | 10, 20, …, 90, 10, 20, … | About 100 ms |

`mode` records the selected pattern, `duty` the current percentage,
`direction` whether the first pattern is rising or falling, and `ticks`
counts iterations. Each iteration delays 10 ms and reads the two buttons.
Two matching consecutive readings are required before a changed button
pattern is accepted. Fifty ticks give the slow step; ten give the fast step.

On a change of mode, duty resets to 10%, direction to rising, and the step
counter to zero. Pressing the already selected mode does not restart it.
Neither button or both buttons does not select a new mode.

The PWM carrier itself is approximately `16000000/(256*256) = 244.14 Hz`.
This is separate from the 500/100 ms intervals at which its duty changes.
An LED shows a slow brightness rise/fall or a quicker rise followed by a drop.
The first pattern returns from 10% to 10% in about 8 s; the second repeats
in about 0.9 s. Timing includes small software overhead.

Compared with the PDF, the buttons remain responsive during the pattern,
and the 90% and 10% endpoints are not held for duplicate steps.

Build/load: `make EX=pwm_two_button_ramp_modes load`.

## 10. INT1 and ADC

Source: [int1_adc_1600mv_leds.c](int1_adc_1600mv_leds.c). PDF pp. 19–20.

**Task:** on an INT1 event, measure ADC1, compare it with approximately
1600 mV, and display the result on LEDs for 2 s.

| Pin | Role |
|---|---|
| PD3 / INT1 | Trigger input with pull-up, rising edge |
| PC1 / ADC1 | Analog voltage input, referenced to AVCC |
| PB5 | Measurement/display active |
| PB0 | Measured value at or above the threshold |
| PB1 | Measured value below the threshold |

With a button to GND, pressing makes PD3 low and **releasing makes the
rising edge**. Press and release to trigger. Merely changing the analog
voltage does not initiate a new measurement.

The interrupt handler disables further INT1 interrupts and sets the volatile
`pending` flag. The main loop notices it, lights PB5, performs one 10-bit
ADC conversion and selects PB0 or PB1. After 2 s it clears all three LEDs,
discards pending INT1 edges and re-enables INT1.

The assumed conversion is `millivolts = raw * 5000 / 1024`. Rather than
calculating a float, the code compares `raw * 5000 >= 1600 * 1024`.
The 32-bit multiplication prevents overflow of AVR's 16-bit arithmetic.
Raw 327 selects PB1; raw 328 selects PB0. Actual analog thresholds depend
on the real AVCC and ADC accuracy.

Test with about 1 V at PC1, then trigger: PB5+PB1 for 2 s. With about 2 V,
trigger again: PB5+PB0 for 2 s. The analog voltage must remain within the
board's valid input range and share its ground.

The ADC value is sampled near the beginning, not at the end of the 2 s.
Events during the indication are ignored; there is no event queue. The
2 s work occurs in main, not inside the interrupt handler.

Build/load: `make EX=int1_adc_1600mv_leds load`.

## 11. Six-bit input, 2024

Source: [exam2024_six_bits_binary_popcount_lcd.c](exam2024_six_bits_binary_popcount_lcd.c).
PDF pp. 20–22.

**Task:** read PB0–PB5, place those six bits in bits 7–2 of an eight-bit
value, and display that value in binary. On line 2, display the binary
value of its popcount plus `0x1E` (decimal 30). Both start at column 5.

The operation `(PINB & 0x3F) << 2` first removes unrelated bits and then
moves the six retained bits left by two. The bottom two bits are zero:

```text
Source: PB5 PB4 PB3 PB2 PB1 PB0
Stored: PB5 PB4 PB3 PB2 PB1 PB0 0 0
```

There is no inversion: released pull-up inputs read as ones. The displayed
popcount is therefore the count of **high electrical levels**, not pressed buttons.

| Raw PB5…PB0 | Stored byte / line 1 | Count | Count+30 / line 2 |
|---|---|---|---|
| `000000` | `00000000` | 0 | `00011110` |
| `000001` | `00000100` | 1 | `00011111` |
| `001011` | `00101100` | 3 | `00100001` |
| `111111` | `11111100` | 6 | `00100100` |

For the third row, the visible screen is:

```text
Line 1: ····00101100····
Line 2: ····00100001····
```

The first sample happens immediately after setup. Timer1 then sets a compare
flag every `(54687+1)*1024/16000000 = 3.500032 s`. The main loop waits for
that flag, clears it by writing 1, and takes the next sample. This version
does no other work while waiting. It is not using a Timer1 interrupt.

Test: leave all six inputs open → `11111100` and `00100100`; connect them
all to GND → `00000000` and `00011110` after the next sampling interval.
The MSB-first printer fixes the PDF's off-by-one bit loop.

Build/load: `make EX=exam2024_six_bits_binary_popcount_lcd load`.

## 12. Logic, shift 1/2025

Source: [exam2025_shift1_logic_lcd.c](exam2025_shift1_logic_lcd.c). PDF pp. 22–25.

**Task:** sample active-low PB0–PB3, show A B C D on line 1 and the two
computed Boolean results F1 F2 on line 2. Both start at column 3.

```text
A=PB0, B=PB1, C=PB2, D=PB3; pressed means logical 1.
F1 = (!B OR D) OR (C AND !A)
F2 = (A AND !B) OR NOT(!A OR D)
   = A AND (!B OR !D)
```

The masked input byte holds A in bit0 through D in bit3. The low-first
printer displays A first. F1 and F2 are sent individually as ASCII digits.
After each update, `_delay_ms(5000)` holds the values for about 5 s.
A short press entirely between samples can be missed.

| Pressed inputs | Line 1 content | Line 2 content |
|---|---|---|
| None | `0000` | `10` |
| A only | `1000` | `11` |
| B only | `0100` | `00` |
| A, B, D | `1101` | `10` |
| All four | `1111` | `10` |

For A only the actual rows begin `··1000` and `··11`.
Hold each test state for more than 5 s to observe it reliably.

The formulas above are the PDF's executable expressions. Its surrounding
comments disagree in places, so this is not proof of the original examiner's
intended formula. The unused/incomplete initialization in the PDF has been
replaced by the working common LCD initialization.

Build/load: `make EX=exam2025_shift1_logic_lcd load`.

## 13. Logic, shift 2/2025

Source: [exam2025_shift2_logic_lcd.c](exam2025_shift2_logic_lcd.c). PDF pp. 25–27.

**Task:** a second pair of logic functions, with inputs shifted one pin
higher than shift 1, displayed from column 6 and updated about every 3 s.

```text
A=PB1, B=PB2, C=PB3, D=PB4; pressed means logical 1.
F1 = (!A AND D) OR (B AND !C)
F2 = (B OR !A) OR NOT(!C OR A)
   = B OR !A
```

The simplification of F2 follows because `NOT(!C OR A) = C AND !A`,
which adds no cases beyond the existing `!A` term.
This is useful when manually checking the display.

`(~PINB >> 1) & 0x0F` moves PB1 into bit0, so the common low-first printer
again produces A B C D. The cursor commands are `0x85` and `0xC5`.

| Pressed inputs | Line 1 content | Line 2 content |
|---|---|---|
| None | `0000` | `01` |
| A only (PB1) | `1000` | `00` |
| B only (PB2) | `0100` | `11` |
| D only (PB4) | `0001` | `11` |
| All four | `1111` | `01` |

With B only, the lines begin `·····0100` and `·····11`.
Hold the input pattern for more than 3 s. Changing PB0 does nothing because
it is outside this exercise's mask. The visible input order is A→D, not D→A.

Build/load: `make EX=exam2025_shift2_logic_lcd load`.

## 14. INT0 and ADC, 2025

Source: [exam2025_shift3_int0_adc_1450mv.c](exam2025_shift3_int0_adc_1450mv.c).
PDF pp. 27–29.

**Task:** trigger on INT0, sample ADC1, light PB5 for 2 s, and then indicate
whether the sampled voltage reached approximately 1450 mV using PB3.

| Pin | Role |
|---|---|
| PD2 / INT0 | Rising-edge trigger; button release with pull-up |
| PC1 / ADC1 | Analog voltage to measure |
| PB5 | Busy indication during the 2 s interval |
| PB3 | Latest completed comparison result |

The ISR disables INT0 and sets `pending`. Main lights PB5, samples once,
waits 2 s, clears PB5 and PB3, then sets PB3 if the saved value passed the
threshold. Finally it discards intervening edges and re-enables INT0.

This source uses `raw*5000/1023`, so the integer comparison is
`raw*5000 >= 1450*1023`. Raw 296 is below, raw 297 is above the threshold.
AVCC is assumed to be 5 V, as in the PDF.

Unlike exercise 10, the result is displayed **after** the wait and remains
until the next completed measurement. During the next busy interval PB3
keeps its previous result; the program does not clear it at the start.

Test: use about 2 V and trigger → PB5 lights, then after 2 s PB5 goes off
and PB3 lights. Change to about 1 V without triggering → PB3 stays on.
Trigger again → after 2 s PB3 goes off. Changes to the voltage during the
wait do not change the already captured sample.

The PDF configured PD3 despite enabling INT0; the code here uses PD2.
It also actually clears PB3 below threshold; `PORTB |= 0` would do nothing.
There is no LCD, and events during the busy interval are not queued.

Build/load: `make EX=exam2025_shift3_int0_adc_1450mv load`.

## 15. Combined exam

Source: [epan25.c](epan25.c). This is the earlier exercise from the supplied image,
separate from the PDF's 14 programs.

**Task:** perform four operations together: sample PB2–PB5 every 3 s, light
PC0 when the sampled value is zero, show the inputs on the LCD, and track
the two latest keypad characters, lighting PC1 when they match.

| Part | Implementation |
|---|---|
| A | PB2–PB5 into `dig_inp` bits 4–7; PC0 reflects `dig_inp == 0` |
| B | First LCD line shows PB2, PB3, PB4, PB5 in that order |
| C | Previous ASCII key in R17, newest in R18; equal pair lights PC1 |
| D | Second LCD line shows those two characters, older first |

For A, `(PINB & 0x3C) << 2` gives PB2→bit4 through PB5→bit7. The lower
four bits are zero. These are raw levels, not inverted button states.
PC0 therefore lights only when **all four inputs are low**. With pull-ups
and all buttons released, the displayed input is `1111` and PC0 is off.

The first line prints bits 4,5,6,7 in that order. If only PB2 is high,
`dig_inp=0x10`, but the LCD reads `1000`: it is a pin-order display, not
MSB-first printing of the stored byte.

Timer1 has OCR1A=46874 and prescaler /1024:
`(46874+1)*1024/16000000 = 3 s`. There is an initial sample during startup.
Thereafter the main loop checks the timer flag and samples when due.
Between those checks it keeps scanning the keypad, rather than delaying for 3 s.
Keypad debounce and LCD work add a small delay before a due sample is handled.

For every newly detected key, the old newest key moves into `previous_key`
and the new ASCII byte becomes `newest_key`. `key_count` saturates at 2;
it prevents the initial zero values being mistaken for a matching pair.

| Press sequence | R17 | R18 | Line 2 | PC1 |
|---|---|---|---|---|
| None | 0 | 0 | Blank | Off |
| `5` | 0 | ASCII `5` | `·5` | Off |
| Release, then `5` again | ASCII `5` | ASCII `5` | `55` | On |
| Then `9` | ASCII `5` | ASCII `9` | `59` | Off |
| Then `9` again | ASCII `9` | ASCII `9` | `99` | On |

Holding 5 is not equivalent to pressing it twice. Use one key at a time,
releasing it long enough for the scan to detect release before pressing again.
PC1 retains its comparison result until another recognized press.

R17 and R18 are real reserved AVR registers here, not ordinary variable names.
The Makefile applies `-ffixed-r17 -ffixed-r18` to both this program and
`ntuaboard.c` to protect them from normal register allocation. Keep those
options when building this exercise. They are not required by the other programs.

| Command | What runs |
|---|---|
| `make EX=epan25 load` | Complete A–D, output `out/epan25.hex` |
| `make EX=full load` or `make load` | Complete A–D, output `out/full.hex` |
| `make EX=a_b load` | Inputs/PC0/LCD line 1 only |
| `make EX=c_d load` | Keypad/PC1/LCD line 2 only |

Test A+B first with known high/low inputs, C+D next with the sequence above,
then the complete program. In full mode, press keys between input sampling
instants to see that both activities continue.

## Interpreting unexpected results

If the wrong exercise seems to be running, check the **EX argument and the
printed `.hex` filename**, not the selected editor tab. Plain `make load`
always selects the combined earlier exam.

If the LCD is blank, start with `name_lcd_third_column` to separate LCD
wiring/contrast problems from input logic. Check that the LCD is connected to
PORTD as required by this library. Neither ADC/interrupt example nor PWM
example is supposed to display anything on the LCD.

If an answer seems inverted, distinguish the physical pin value from the
logical pressed value. In most button exercises pressed means logical 1;
in the two raw-input exercises a button to GND makes the shown bit 0.

If an input seems slow, check the program's sampling interval. The 2024
exercise waits 3.5 s; the 2025 logic exercises wait 5 s or 3 s. Those
programs can miss short presses between samples. The two interrupt examples
trigger on button release and ignore further events during their 2 s window.

For an ISP programming failure, `make load` now automatically checks whether
the board was left in debugWIRE mode and restores ISP when that is confirmed.
It reads the current fuse, changes only DWEN, and verifies the result before
programming. Connect one board at a time. Hardware/power faults still require
attention; this recovery only addresses the debugWIRE/ISP mode mismatch.
If using Microchip Studio separately, finish debugging with its documented
“Disable debugWIRE and Close” procedure rather than guessing fuse bytes.
The [setup notes](../../SETUP.md) describe the project's programmer setup.
