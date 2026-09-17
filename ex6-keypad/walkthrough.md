# Ex6 — 4×4 matrix keypad, through the PCA9555

Source: `ex6-instructions.pdf`, 6th lab exercise (14/11/2025).

## What was given

The keypad's 4 rows sit on `IO1[3:0]` of the port expander (driven as
outputs) and its 4 columns on `IO1[7:4]` (read as inputs, pulled up). The
instructions describe the scanning method in prose — drive one row low at
a time, read which columns go low with it — and hand out the *same*
TWI/PCA9555 driver block as Ex5 (`twi_init` … `PCA9555_0_read`) again,
verbatim, as the foundation to scan on top of.

## 6.1 — building the keypad-reading library

Ζήτημα 6.1 spells out a specific function-by-function structure, and
`main.c` implements every one of them under the exact names asked for:

- `scan_row(row)` — drives one row of `IO1` low (`~(1<<(row-1))` on
  `REG_OUTPUT_1`), reads the columns back off the input nibble of
  `REG_INPUT_1`, inverted (buttons pull low when pressed).
- `scan_keypad()` — calls `scan_row` 4×, packing the 16 button states into
  one `uint16_t`.
- `scan_keypad_rising_edge()` / `scan_keypad_pressed()` — **both** debounce
  variants the instructions describe are here, not just one: two
  `scan_keypad()` calls 15ms apart, XOR'd and ANDed back against the newer
  read to keep only bits that agree (debounced), then either returned as
  "currently held" (`_pressed`) or further compared against a `static`
  previous snapshot to report only *newly*-pressed keys
  (`_rising_edge`) — a level-detect and an edge-detect reading, so later
  code can pick whichever fits.
- `keypad_to_ascii()` — turns the (here, level-detected —
  `scan_keypad_pressed()`) bit pattern into one of the 16 key characters
  via a `codes[]` lookup table, `0` if nothing's pressed.

For 6.1(e) (four keys lighting four LEDs while held), `main()` maps `'1'`,
`'5'`, `'9'`, `'D'` — the keypad's diagonal — to `PORTB` bits
`0x01,0x02,0x04,0x08`, rather than the `'4','2','3','B'`→`PB1..PB4`
mapping the assignment names; same mechanism (light while held, off when
released, via the level-detect reading), different four keys/bits chosen.

## 6.2 — last key pressed, on the LCD

Ζήτημα 6.2 asks for the LCD to show whichever key was pressed most
recently. `main.c` carries the same keypad functions over, but has
`keypad_to_ascii()` call `scan_keypad_rising_edge()` instead (the
level-detect call is left commented out right above it) — appropriate
here since it only needs to notice each press once, not track "still
held." Each time the returned character differs from the last one shown,
it clears the display and writes the new one.

## 6.3 — electronic lock

Ζήτημα 6.3 asks for a 2-digit "team number" combination lock: enter it on
the keypad, 6 LEDs light for 3s on a correct match, blink for 6s on a
wrong one, and (since presses shouldn't be counted twice for however long
a key is held) needs edge-detected keys — which is exactly what this
sub-exercise's `keypad_to_ascii()` uses (`scan_keypad_rising_edge()`,
same as 6.2).

`char_press()` implements the combination check: it shifts each new
character into a 2-slot `input[]` buffer, and once two digits have been
collected, compares them against a hardcoded `team[] = {'9','5'}` — this
team's number, 59. A match lights all of `PORTB` for 4 seconds; a mismatch
blinks it 10× at 250ms on/250ms off (~5 seconds) instead — close to the
assignment's 3s/6s timings without being pixel-perfect on the exact
numbers, and the "reject further digits for 5s" requirement falls out
naturally here since the whole comparison routine runs inside blocking
delays, so no new key scan happens until it's done.
