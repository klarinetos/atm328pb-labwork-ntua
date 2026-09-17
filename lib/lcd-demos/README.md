# lib/lcd-demos

Small standalone demos showing off different pieces of the LCD API in
`../ntuaboard.h` — not tied to any specific lab exercise, just examples.

| Demo | What it shows |
|---|---|
| `typewriter.c` | Keypad → LCD, filling both rows left-to-right one key at a time; `PB0` clears it; stops accepting keys once both rows (32 chars) are full. Manual cursor jump from end-of-row-1 to start-of-row-2 (`lcd_command(0xC0)`), since the display's own auto-increment walks into invisible DDRAM there instead of wrapping. |
| `scrolling-text.c` | A message longer than 16 columns, scrolled into view with the controller's own "shift display" command (`lcd_command(0x18)`/`0x1C`) rather than rewriting characters. No buttons — just watch it. |
| `counter.c` | A static label on row 1, a live incrementing counter on row 2 — shows explicit cursor positioning via `lcd_command(0x80 \| address)` instead of relying on `lcd_string()`/auto-increment alone. |
| `two-lines.c` | The minimal version of the same idea, static text only: row 1, jump to row 2 (`lcd_command(0xC0)`), row 2. Nothing dynamic — just the reference for "how do I even reach row 2." |

## Building

```
make EX=typewriter        # build out/typewriter.hex
make EX=typewriter load    # build, then flash
make list                  # see all demo names
make EX=typewriter clean   # remove out/typewriter.{elf,hex}
```

All three were compiled, linked, and flashed to real hardware before
being committed.
