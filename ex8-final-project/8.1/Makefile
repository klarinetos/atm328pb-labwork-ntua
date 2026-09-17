# Build + flash this main.c on an ATmega328PB Xplained Mini.
# Run `make` to build, `make load` to build then flash. See CLAUDE.md at the
# repo root for why these exact tools/flags are used (in particular -O1 and
# -Wno-error, both load-bearing, not stylistic).

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

XC8CC       := C:/Program Files/Microchip/xc8/v3.10/bin/xc8-cc.exe
OBJCOPY     := C:/Program Files/Microchip/xc8/v3.10/bin/avr-objcopy.exe
DFP         := C:/Users/sgkio/.mchp_packs/Microchip/ATmega_DFP/3.6.299/xc8
AVRDUDE     := C:/Users/sgkio/AppData/Local/Microsoft/WinGet/Packages/AVRDudes.AVRDUDE_Microsoft.Winget.Source_8wekyb3d8bbwe/avrdude.exe
RM          := C:/Program Files/Microchip/MPLABX/v6.35/gnuBins/GnuWin32/bin/rm.exe

CFLAGS := -mcpu=ATmega328PB -x c -mdfp="$(DFP)" -O1 -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -Wall -Wno-error -gdwarf-3 -mno-const-data-in-progmem

OUT := out
ELF := $(OUT)/main.elf
HEX := $(OUT)/main.hex

.PHONY: all build load clean

all: build

build: $(HEX)

$(ELF): main.c
	"$(XC8CC)" $(CFLAGS) main.c -o "$(ELF)"

$(HEX): $(ELF)
	"$(OBJCOPY)" -O ihex "$(ELF)" "$(HEX)"

load: build
	"$(AVRDUDE)" -c xplainedmini_isp -p m328pb -U flash:w:"$(HEX)":i

clean:
	-"$(RM)" -f "$(OUT)/main.elf" "$(OUT)/main.hex"
