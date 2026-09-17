# Ex8 — Final project: a tiny "hospital IoT" node

Source: `ex8-instructions.pdf`, 8th lab exercise (28/11/2025) — explicitly a
"συνδυαστική/επαναληπτική" (combinational/review) exercise: it's meant to
pull together everything from Ex5–Ex7 (TWI/PCA9555, keypad, LCD, DS18B20)
plus a new piece, USART, behind a simulated hospital-bed sensor node that
talks to a pre-built Gateway over WiFi.

## What was given

- **USART theory + a given init/transmit/receive layer**: `usart_init`
  (9600 baud, 8N1, `UBRR` computed from `F_CPU`/baud), `usart_transmit`
  (spin on `UDRE0`), `usart_receive` (spin on `RXC0`) — the basis every
  `USART_Init`/`usart_transmit`/`usart_receive` here is built from.
- **The ESP8266 command protocol**: a fixed text command set the ESP
  module accepts over that same USART (`ESP:connect`, `ESP:url:"..."`,
  `ESP:payload:[...]`, `ESP:transmit`, …), each command needing a trailing
  `\n` and answered with a line back (`"Success"`/`"Fail"` or a proper
  HTTP-style response for `transmit`). SSID/password/baud rate are
  pre-configured on the ESP itself.
- **The scenario** (Εικόνα 2): each `ntuAboard` is one hospital-bed "IoT
  node," reporting a simulated patient temperature (the real DS18B20 from
  Ex7) and a simulated venous-pressure sensor (a potentiometer standing in
  for a real 0–20 cmH₂O sensor) to a Gateway, which the course already
  runs for you.

## 8.1 — talking to the ESP, showing each step on the LCD

Ζήτημα 8.1 asks for `ESP:connect` then `ESP:url:"http://192.168.1.250:5000/data"`,
each shown on the LCD as a numbered step (`1.Success`/`1.Fail`, then
`2.…`).

`main.c` implements the USART/ESP layer (`usart_transmit_str` appends the
required `\n`; `usart_receive_str` reads back a line up to it and hands it
to `lcd_string`; `usart_command` chains send-then-receive) and drives
exactly this numbered-step pattern (`lcd_data('1'); lcd_data('.');
usart_command("ESP:connect"); …`) — though it goes a bit further than 8.1
literally asks, running all the way through steps `3.` (`ESP:payload:...`)
and `4.` (`ESP:transmit`) too, and always shows the ESP's actual response
text rather than collapsing it to just "Success"/"Fail" — closer to what
8.3 later asks of step 4 specifically, applied to every step here.

## 8.2 — adding the two sensors and a status

Ζήτημα 8.2 asks the program to also read the DS18B20 (offset near 36°C to
look like a real patient reading) and `POT0` (rescaled 0–20 cmH₂O), and
derive a `status` string: `"CHECK TEMP"` outside 34–37°C, `"CHECK
PRESSURE"` outside 4–12 cmH₂O, `"NURSE CALL"` on a specific keypad
sequence, else `"OK"`.

`main.c` reads `therm_read_temperature()` twice per loop (the first read
discarded — likely to skip a stale conversion), adds a fixed offset, and
flags `statuses[1]` ("CHECK TEMP") when the scaled result falls outside
**340–370** (i.e. 34.0–37.0°C). It reads `ADCH` from `POT0` and flags
`statuses[2]` ("CHECK PRESSURE") outside **51–153** counts — the 0–255
range that maps to the assignment's 4–12 cmH₂O window once scaled to
0–20. Both messages are shown on the LCD each pass, 1s and 2s apart.
**The keypad-driven "NURSE CALL" status from 8.2(c) isn't implemented in
this file** — there's no keypad reading here at all yet; that piece
arrives (partially) in 8.3.

## 8.3 — sending it to the Gateway

Ζήτημα 8.3 asks for the full payload — `[{"name":"temperature",...},
{"name":"pressure",...},{"name":"team","value":"5"},{"name":"status",...}]`
— sent via `ESP:payload:...` then `ESP:transmit`, with the server's actual
response (not a canned Success/Fail) shown as step `4.` on the LCD.

`main.c` builds exactly that JSON with `sprintf` (temperature and pressure
each formatted to one decimal place from the raw sensor values, `"team":
"59"` hardcoded — this team's number, matching the `59` combination from
Ex6's electronic lock), sends it as step `3.`, then `ESP:transmit` as step
`4.`, displaying whatever the Gateway actually replies with either time —
matching the assignment precisely for the temperature/pressure/transmit
half.

**The "NURSE CALL" status is still not reachable, though the scaffolding
for it is now present**: `statuses[]` gets a 4th entry (`"NURSECALL"`),
`keypad_to_ascii()` is defined (same function as Ex6), and a `key_pressed`
variable is declared — but `main()` never actually calls
`keypad_to_ascii()` or assigns anything based on it, so `status` can only
ever land on `"OK"`, `"CHECKTEMP"`, or `"CHECKPRESSURE"` at runtime. (One
small side effect of adding that 4th string to a `statuses[3][20]` array
still declared as 3 rows: a few compilers would flag "excess elements in
array initializer" — this build only warns, per this repo's `-Wno-error`,
and since that 4th status is never read back out, it doesn't affect
anything that actually runs.)
