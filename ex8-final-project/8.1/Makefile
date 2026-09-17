# Build + flash this main.c on an ATmega328PB Xplained Mini.
# Run `make` to build, `make load` to build then flash. See CLAUDE.md at the
# repo root for why these exact tools/flags are used (in particular -O1 and
# -Wno-error, both load-bearing, not stylistic).

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

AVRGCC      := C:/Program Files (x86)/Microchip/xc8/v2.10/avr/bin/avr-gcc.exe
OBJCOPY     := C:/Program Files (x86)/Microchip/xc8/v2.10/avr/bin/avr-objcopy.exe
V210DFP     := C:/Program Files (x86)/Microchip/xc8/v2.10/dfp
AVRDUDE     := avrdude

CFLAGS := -mmcu=atmega328pb -D__AVR_DEV_LIB_NAME__=m328pb "-I$(V210DFP)/include" "-B$(V210DFP)/gcc/dev/atmega328pb/avr5" -O1 -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -Wall -Wno-error -gdwarf-3 -mno-const-data-in-progmem

OUT := out
ELF := $(OUT)/main.elf
HEX := $(OUT)/main.hex

.PHONY: all build load clean

all: build

build: $(HEX)

$(ELF): main.c
	"$(AVRGCC)" $(CFLAGS) main.c -o "$(ELF)"

$(HEX): $(ELF)
	"$(OBJCOPY)" -O ihex "$(ELF)" "$(HEX)"

load: build
	"$(AVRDUDE)" -c xplainedmini_isp -p m328pb -U flash:w:"$(HEX)":i

clean:
	-del /f /q $(subst /,\,$(OUT)/main.elf $(OUT)/main.hex) >nul 2>&1
