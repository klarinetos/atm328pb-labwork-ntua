# lcd-demos

Small standalone demos showing off different pieces of the LCD API in
`ntuaboard.h` — not tied to any specific lab exercise, just examples.

This folder is fully self-contained: `ntuaboard.h`/`ntuaboard.c` here are
copies of `../lib/ntuaboard.h`/`.c`, not a reference to `lib/` — on
purpose, so this folder doesn't depend on anything outside itself. That
does mean a fix made in one copy (like the `lcd_command()` timing fix)
doesn't automatically apply to the other; check both if you touch the LCD
driver.

| Demo | What it shows |
|---|---|
| `typewriter.c` | Keypad → LCD, filling both rows left-to-right one key at a time; `PB0` clears it; stops accepting keys once both rows (32 chars) are full. Manual cursor jump from end-of-row-1 to start-of-row-2 (`lcd_command(0xC0)`), since the display's own auto-increment walks into invisible DDRAM there instead of wrapping. |
| `typewriter-1row.c` | The simpler version of the above — same keypad-fills-the-LCD idea and `PB0` reset, but only row 1 (16 characters), no cursor jump at all. |
| `scrolling-text.c` | A message longer than 16 columns, scrolled into view with the controller's own "shift display" command (`lcd_command(0x18)`/`0x1C`) rather than rewriting characters. No buttons — just watch it. |
| `counter.c` | A static label on row 1, a live incrementing counter on row 2 — shows explicit cursor positioning via `lcd_command(0x80 \| address)` instead of relying on `lcd_string()`/auto-increment alone. |
| `two-lines.c` | The minimal version of the same idea, static text only: row 1, jump to row 2 (`lcd_command(0xC0)`), row 2 — each line horizontally centered, showing how to combine the row jump with a column offset. |
| `adc-voltage-disp.c` | Reads ADC2, prints it as a centered `"D.DD V"` voltage reading once a second — adapted from `lib/examples/4.2.c`, with the ADC-scaling and centering math spelled out in comments since it's meant to be read, not just run. |

## Building

```
make EX=typewriter        # build out/typewriter.hex
make EX=typewriter load    # build, then flash
make list                  # see all demo names
make EX=typewriter clean   # remove out/typewriter.{elf,hex}
```

Every demo here was compiled, linked, and flashed to real hardware before
being committed.
