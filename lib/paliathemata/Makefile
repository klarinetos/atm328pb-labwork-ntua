# Build one program: make EX=filename_without_c. Default: full (epan25).
# Add load to flash the selected program; see README.md for wiring.

SHELL       := C:/Windows/System32/cmd.exe
.SHELLFLAGS := /C

XC8CC       := C:/Program Files/Microchip/xc8/v2.36/bin/xc8-cc.exe
OBJCOPY     := C:/Program Files/Microchip/xc8/v2.36/bin/avr-objcopy.exe
DFP         := C:/Program Files/Microchip/xc8/v2.36/dfp/xc8
ATPROGRAM   := C:/Program Files (x86)/Atmel/Studio/7.0/atbackend/atprogram.exe
RM          := C:/Program Files (x86)/Atmel/Studio/7.0/shellutils/rm.exe

CFLAGS := -mcpu=ATmega328PB -x c -mdfp="$(DFP)" -O1 -ffunction-sections -fdata-sections -fshort-enums -funsigned-char -funsigned-bitfields -Wall -Wextra -Wno-error -gdwarf-3 -mno-const-data-in-progmem

EX ?= full
PROGRAMS := $(basename $(notdir $(wildcard *.c)))

ifneq ($(filter $(EX),full a_b c_d),)
SOURCE := epan25.c
ifeq ($(EX),a_b)
CFLAGS += -DENABLE_KEYS=0
else ifeq ($(EX),c_d)
CFLAGS += -DENABLE_INPUTS=0
endif
else
ifeq ($(filter $(EX),$(PROGRAMS)),)
$(error Unknown EX=$(EX). Run make list)
endif
SOURCE := $(EX).c
endif

# Only the earlier exam explicitly requires physical R17/R18 storage.
ifeq ($(SOURCE),epan25.c)
CFLAGS += -ffixed-r17 -ffixed-r18
endif

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

build: $(HEX)

$(OUT)/%.elf: $(SOURCE) exam_helpers.h ../ntuaboard.c ../ntuaboard.h Makefile
	@if not exist "$(OUT)" mkdir "$(OUT)"
	"$(XC8CC)" $(CFLAGS) $(SOURCE) ../ntuaboard.c -o "$@"

$(OUT)/%.hex: $(OUT)/%.elf
	"$(OBJCOPY)" -O ihex "$<" "$@"

# Pass an absolute Windows short path: atprogram cannot decode Greek path characters.
load: build
	for %%I in ("$(HEX)") do powershell.exe -NoProfile -ExecutionPolicy Bypass -File "../../tools/Load-Avr.ps1" -Atprogram "$(ATPROGRAM)" -HexPath "%%~fsI"

clean:
	-"$(RM)" -f "$(OUT)"/*.elf "$(OUT)"/*.hex

list:
	@echo $(PROGRAMS) full a_b c_d
