# microcontrollers

AVR (ATmega328PB) exercises for a "Microcontrollers" university lab course,
flashed to an **ATmega328PB Xplained Mini** board.

## Setup (one-time)

Three tools, none of which need MPLAB X (the IDE) installed at all:

1. **XC8 v2.10** — the AVR compiler toolchain (`avrasm2`, `avr-gcc`,
   `avr-objcopy`) plus its bundled ATmega328PB device support. Not on
   winget — download the Windows installer from Microchip's site
   ([XC8 downloads](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-xc-compilers) →
   look under "all downloads"/the version archive for **v2.10**, not the
   latest version — this repo specifically needs v2.10, see
   [CLAUDE.md](CLAUDE.md) for why). Default install path (used everywhere
   in this repo) is `C:\Program Files (x86)\Microchip\xc8\v2.10\`.

2. **`make`** — GNU Make, via winget:
   ```
   winget install ezwinports.make
   ```

3. **`avrdude`** — flashes the board, via winget:
   ```
   winget install AVRDudes.AVRDUDE
   ```

After the winget installs, **restart your terminal** (or VS Code
entirely) — Windows only hands a freshly-started process the updated
`PATH`, so a terminal that was already open won't see either tool until
restarted. Confirm both landed with `make --version` and `avrdude -v`.

**Board**: connect the Xplained Mini via USB — its onboard debugger
(mini-EDBG chip) is both the programmer and the target, so that's the
only cable needed, no separate programmer. If `avrdude` can't find it,
Windows may need a moment (or a replug) to enumerate the board's USB
interfaces; a driver has never been an issue on any machine this was
tested on, but if one ever is needed, it ships with
[Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio)
(no need to install the whole thing — the driver package alone will do).

**Optional**: VS Code, only if you want the `Ctrl+Shift+B` / Run Task
interface instead of typing `make` — see [Building & flashing](#building--flashing)
below, both are fully equivalent.

## Layout

One folder per exercise, one subfolder per standalone program inside it:

```
ex1-intro/
  ex1-instructions.pdf   (professor's assignment sheet)
  Report.pdf             (lab report for exercise 1, not pushed)
  1.1/  main.asm
  1.2/  main.asm
  1.3/  main.asm
ex2-interrupts/
  ex2-instructions.pdf
  Report.pdf
  2.1/       main.asm
  2.2/       main.asm
  2.3_asm/   main.asm   (asm version of 2.3)
  2.3_C/     main.c     (C version of 2.3)
ex3-adc-pwm/     ...
ex4-adc-lcd/     ...
ex5-i2c/         ...
ex6-keypad/      ...
ex7-temp-sensor/ ...
ex8-final-project/
  ex8-instructions.pdf
  Report.pdf
  8.1/ 8.2/ 8.3/  main.c

lib/             (shared driver library used by the exercises above + worked examples)
lcd-demos/       (standalone LCD demos, not tied to any exercise)
```

Each sub-exercise folder is a **self-contained project**: open it directly
in VS Code (`File > Open Folder`) to get its own build/flash tasks.

## Building & flashing

Each project folder has two equivalent ways to build/flash it:

- **VS Code**: open that specific subfolder (not the repo root), then
  `Ctrl+Shift+B` builds it, and Terminal → Run Task →
  **"avrdude: flash to ATmega328PB Xplained Mini"** flashes it.
- **Command line**: `make` builds it, `make load` builds then flashes it,
  `make clean` removes build output. (Needs the one-time [Setup](#setup-one-time)
  above done first.)

See [CLAUDE.md](CLAUDE.md) for exactly how this works, why it's set up this
way, and the one known exception (`ex7-temp-sensor/7.1` is a helper-function
fragment with no `main()`, not a standalone program).
