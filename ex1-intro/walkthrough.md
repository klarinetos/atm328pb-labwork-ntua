# Ex1 — Intro: delay routines, logic functions, and a "shuttle" LED

Source: `ex1-instructions.pdf`, 1st lab exercise (10/10/2025).

## What was given

The instructions hand you a **skeleton**, not full code — just the shape a
delay routine must have:

```asm
rcall wait_x_msec
...
wait_x_msec:
....
ret
```

with the constraint that `rcall wait_x_msec` must take **exactly x
milliseconds**, x coming in as a 16-bit value split across `r24`
(low)/`r25` (high), and that the timing loop must use `sbiw` (the only
register-pair instruction in the mix) rather than two separate 8-bit
decrements. `docs/NtuaBoard_G1.pdf`'s own generic assembly example (the
`delay_inner`/`delay_outer` pair on its last two slides) shows the general
double-nested-loop pattern this is built from: an outer loop counting `x`
down via `sbiw`, each iteration of which burns a fixed number of cycles in
an inner loop.

## 1.1 — the delay routine itself

`main.asm` implements exactly that skeleton, under the name `delay_mS`
rather than `wait_x_msec`:

- `loop_inn1`/`delay_inner`: an inner loop (`dec r23` / `nop` / `brne`)
  tuned to burn a fixed cycle count per pass.
- `delay_mS`: the outer loop — `sbiw r24,1` / `brne delay_mS` — that repeats
  the inner loop `x` times, where `x = r25:r24` on entry, satisfying the
  "duration proportional to the r25:r24 pair, using `sbiw`" requirement.

The rest of the file is a minimal demo program built around it: it drives
`PORTD` high/low with `com`/`rcall delay_mS` in `loop1`, blinking all of
PORTD's LEDs at the rate set by `FOSC_MHZ`/`DEL_mS` — enough to watch the
delay routine work under the MPLAB X simulator's Stopwatch tool, per the
instructions.

## 1.2 — combinational logic functions

Ζήτημα 1.2 asks for two boolean functions of four one-byte variables
A, B, C, D, each incremented by a different fixed step every pass of a
loop, simulated to fill in a results table by hand.

**Note:** the instructions PDF states the specific formulas as
`F0 = (A'·B + B'·D)'` / `F1 = (A+C)·(B+D)` with initial values
`A=0x52,B=0x42,C=0x22,D=0x02` incremented by `1/2/3/4` over 6 iterations —
but `main.asm` actually implements a different pair:

```
F0 = (A'B'C' + D)'
F1 = (A'+C)(B'+D')
```

with initial values `A=0x45, B=0x23, C=0x21, D=0x01`, incremented by
`1/2/4/5` respectively over **5** iterations (register aliases `.def
A=r16`, `B=r17`, `C=r18`, `D=r19`, `F0=r20`, `F1=r21`). This looks like a
per-team variant of the assignment (these labs commonly permute the
constants per team) rather than a mismatch worth "fixing" — the mechanism
is identical: each function built from `com`/`and`/`or` on copies of the
inputs, the four variables bumped by their own step count via repeated
`inc` at the bottom of `loop`, `dec r22`/`brne loop` driving the iteration
count.

## 1.3 — the "shuttle wagon"

Ζήτημα 1.3 asks for a single bit walking back and forth across `PORTD`
(LSb→MSb→LSb…), simulating a wagon on a track: one step roughly every 2
seconds, direction tracked in SREG's **T flag**, with an extra 1-second
pause (3s total) each time it reverses at an end — built on top of 1.1's
delay routine.

`main.asm` keeps a single set bit moving through `PORTD`: `lsl r16` walks it
left (`start_loop`) until it falls off the top (`cpi r16,0` after the
shift), then `lsr r16` walks it right (`inner`) until it falls off the
bottom, then back to `lsl`. `set`/`clt` record the current direction in
SREG's **T flag** on each reversal, satisfying that part of the spec, even
though this version doesn't branch on `T` — the left/right code is just two
separate labeled blocks reached by falling through from the shift check,
rather than one shared loop steered by testing `T`. The extra pause at each
end comes from an extra `rcall delay_mS` right at `rotate_right`/before
jumping back to `start`, on top of the 3× `delay_mS` already burned per
normal step. Note the actual `DEL_mS` here (50) makes each step far
quicker than the assignment's "~2 sec" — convenient for watching it move in
the MPLAB X simulator without waiting around, tune it up for real hardware
timing.

Watching `PORTD` live requires MPLAB X's Run Time Watch, as the
instructions point out — nothing observable happens on real hardware
faster than the eye can follow otherwise.
