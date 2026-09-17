# Build + flash this avrasm2-dialect main.asm on an ATmega328PB Xplained Mini.
# Run `make` to build, `make load` to build then flash. See CLAUDE.md at the
# repo root for why these exact tools/flags are used.

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

AVRASM2     := C:/Program Files (x86)/Microchip/xc8/v2.10/avr/bin/avrasm2.exe
AVRASM2_INC := C:/Program Files (x86)/Microchip/xc8/v2.10/dfp/avrasm/inc
AVRDUDE     := avrdude

OUT := out
HEX := $(OUT)/main.hex

.PHONY: all build load clean

all: build

build: $(HEX)

$(HEX): main.asm
	"$(AVRASM2)" -fI -o "$(OUT)/main.hex" -l "$(OUT)/main.lst" -m "$(OUT)/main.map" -I "$(AVRASM2_INC)" main.asm

load: build
	"$(AVRDUDE)" -c xplainedmini_isp -p m328pb -U flash:w:"$(HEX)":i

clean:
	-del /f /q $(subst /,\,$(OUT)/main.hex $(OUT)/main.lst $(OUT)/main.map) >nul 2>&1
