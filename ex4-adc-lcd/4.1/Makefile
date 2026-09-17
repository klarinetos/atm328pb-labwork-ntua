# Build + flash this avrasm2-dialect main.asm on an ATmega328PB Xplained Mini.
# Run `make` to build, `make load` to build then flash. See CLAUDE.md at the
# repo root for why these exact tools/flags are used.

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

AVRASM2     := C:/Program Files (x86)/Microchip/xc8/v2.10/avr/bin/avrasm2.exe
AVRASM2_INC := C:/Program Files (x86)/Microchip/xc8/v2.10/dfp/avrasm/inc
AVRDUDE     := C:/Users/sgkio/AppData/Local/Microsoft/WinGet/Packages/AVRDudes.AVRDUDE_Microsoft.Winget.Source_8wekyb3d8bbwe/avrdude.exe
RM          := C:/Program Files/Microchip/MPLABX/v6.35/gnuBins/GnuWin32/bin/rm.exe

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
	-"$(RM)" -f "$(OUT)/main.hex" "$(OUT)/main.lst" "$(OUT)/main.map"
