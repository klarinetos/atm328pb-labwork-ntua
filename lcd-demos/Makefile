# Build + flash one LCD demo on an ATmega328PB Xplained Mini.
# Run `make EX=typewriter` to build, `make EX=typewriter load` to build
# then flash. `make list` shows the available demo names. See CLAUDE.md
# at the repo root for why these exact tools/flags are used, and see
# README.md here for what each demo does.

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

AVRGCC      := C:/Program Files (x86)/Microchip/xc8/v2.10/avr/bin/avr-gcc.exe
OBJCOPY     := C:/Program Files (x86)/Microchip/xc8/v2.10/avr/bin/avr-objcopy.exe
V210DFP     := C:/Program Files (x86)/Microchip/xc8/v2.10/dfp
AVRDUDE     := avrdude

CFLAGS := -mmcu=atmega328pb -D__AVR_DEV_LIB_NAME__=m328pb "-I$(V210DFP)/include" "-B$(V210DFP)/gcc/dev/atmega328pb/avr5" -O1 -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -Wall -Wextra -Wno-error -gdwarf-3 -mno-const-data-in-progmem

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
	@echo Pass which demo to build, e.g. "make EX=typewriter". Run "make list" to see the names.
	@exit 1
else
	$(MAKE) "$(HEX)"
endif

$(OUT)/%.elf: %.c ntuaboard.c ntuaboard.h
	"$(AVRGCC)" $(CFLAGS) $(EX).c ntuaboard.c -o "$@"

$(OUT)/%.hex: $(OUT)/%.elf
	"$(OBJCOPY)" -O ihex "$<" "$@"

load: build
	"$(AVRDUDE)" -c xplainedmini_isp -p m328pb -U flash:w:"$(HEX)":i

clean:
ifndef EX
	-del /f /q $(subst /,\,$(OUT))\*.elf $(subst /,\,$(OUT))\*.hex >nul 2>&1
else
	-del /f /q $(subst /,\,$(ELF) $(HEX)) >nul 2>&1
endif

list:
	@echo typewriter typewriter-1row scrolling-text counter two-lines
