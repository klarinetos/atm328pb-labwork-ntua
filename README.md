# microcontrollers

AVR (ATmega328PB) exercises for a "Microcontrollers" university lab course,
flashed to an **ATmega328PB Xplained Mini** board.

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
```

Each sub-exercise folder is a **self-contained project**: open it directly
in VS Code (`File > Open Folder`) to get its own build/flash tasks.

## Building & flashing

Each project folder has two equivalent ways to build/flash it:

- **VS Code**: open that specific subfolder (not the repo root), then
  `Ctrl+Shift+B` builds it, and Terminal → Run Task →
  **"avrdude: flash to ATmega328PB Xplained Mini"** flashes it.
- **Command line**: `make` builds it, `make load` builds then flashes it,
  `make clean` removes build output. (One-time setup needed first — see
  CLAUDE.md.)

See [CLAUDE.md](CLAUDE.md) for exactly how this works, why it's set up this
way, the `make` one-time PATH setup, and the one known exception
(`ex7-temp-sensor/7.1` is a helper-function fragment with no `main()`, not a
standalone program).
