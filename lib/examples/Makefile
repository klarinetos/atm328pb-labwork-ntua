# Build + flash one example on an ATmega328PB Xplained Mini.
# Run `make EX=6.3` to build, `make EX=6.3 load` to build then flash.
# `make list` shows the available example names. See CLAUDE.md at the
# repo root for why these exact tools/flags are used (in particular -O1
# and -Wno-error, both load-bearing, not stylistic), and see
# examples/README.md for what each example is.

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

XC8CC       := C:/Program Files/Microchip/xc8/v3.10/bin/xc8-cc.exe
OBJCOPY     := C:/Program Files/Microchip/xc8/v3.10/bin/avr-objcopy.exe
DFP         := C:/Users/sgkio/.mchp_packs/Microchip/ATmega_DFP/3.6.299/xc8
AVRDUDE     := C:/Users/sgkio/AppData/Local/Microsoft/WinGet/Packages/AVRDudes.AVRDUDE_Microsoft.Winget.Source_8wekyb3d8bbwe/avrdude.exe
RM          := C:/Program Files/Microchip/MPLABX/v6.35/gnuBins/GnuWin32/bin/rm.exe

CFLAGS := -mcpu=ATmega328PB -x c -mdfp="$(DFP)" -O1 -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -Wall -Wextra -Wno-error -gdwarf-3 -mno-const-data-in-progmem

OUT := out

.PHONY: all build load clean list
# Without this, make treats %.elf as a disposable intermediate in the
# %.c -> %.elf -> %.hex chain and deletes it right after linking the
# .hex -- keep it around like every other Makefile in this repo does.
.SECONDARY:

ifdef EX
ELF := $(OUT)/$(EX).elf
HEX := $(OUT)/$(EX).hex
endif

all: build

build:
ifndef EX
	@echo Pass which example to build, e.g. "make EX=6.3". Run "make list" to see the names.
	@exit 1
else
	$(MAKE) "$(HEX)"
endif

$(OUT)/%.elf: %.c ../ntuaboard.c ../ntuaboard.h
	"$(XC8CC)" $(CFLAGS) $(EX).c ../ntuaboard.c -o "$@"

$(OUT)/%.hex: $(OUT)/%.elf
	"$(OBJCOPY)" -O ihex "$<" "$@"

load: build
	"$(AVRDUDE)" -c xplainedmini_isp -p m328pb -U flash:w:"$(HEX)":i

clean:
ifndef EX
	-"$(RM)" -f "$(OUT)"/*.elf "$(OUT)"/*.hex
else
	-"$(RM)" -f "$(ELF)" "$(HEX)"
endif

list:
	@echo 3.2 3.3 4.2 4.3 5.1 5.2 6.1 6.2 6.3 7.2 8.1 8.2 8.3
