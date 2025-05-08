MCU_TARGET	= attiny412
BASE		= $(HOME)/extbuilds/avr8-gnu-toolchain-linux_x86_64
CC 		= $(BASE)/bin/avr-gcc
OBJCOPY		= $(BASE)/bin/avr-objcopy
OBJDUMP		= $(BASE)/bin/avr-objdump
SIZE		= $(BASE)/bin/avr-size
PORT		= /dev/ttyUSB0
ARBASE		= $(HOME)/.arduino15/packages
PYMCMU		= $(ARBASE)/megaTinyCore/tools/python3/3.7.2-post1/python3 -u $(ARBASE)/megaTinyCore/hardware/megaavr/2.6.10/tools/prog.py -t uart -u $(PORT) -b 230400 -d $(MCU_TARGET) -a write
WATCHDOG_FUSE	= 0:0b00000000

# run at 16Mhz
OSC_FUSE	= 2:0x01

# nocrc | updi | don't erase eeprom
SYSCFG_FUSE	= 5:0b11000101

# delay 8ms from power to code execution
SYSCFG1_FUSE	= 6:0x04
APPEND_FUSE	= 7:0x00
BOOTEND_FUSE	= 8:0x00


CFLAGS		= -g -Wall -O2 -mmcu=$(MCU_TARGET)

all: main.elf

main.o: main.c dsp.h rotary.h
	$(CC) $(CFLAGS) -c main.c

rotary.o: rotary.c rotary.h
	$(CC) $(CFLAGS) -c rotary.c

button.o: button.c rotary.h
	$(CC) $(CFLAGS) -c button.c

main.elf: main.o rotary.o
	$(CC) $(CFLAGS) $(LDFLAGS)  $(LIBS) -o $@ $^

flash: main.size main.hex
	$(PYMCMU) --fuses \
		${WATCHDOG_FUSE} \
		${OSC_FUSE} \
		${SYSCFG_FUSE} \
		${SYSCFG1_FUSE} \
		${APPEND_FUSE} \
		${BOOTEND_FUSE} \
		-f main.hex -a write

clean:
	rm -f *.o *.elf *.lst *.map *.hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .rodata -j .bss -j .eeprom -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.size: %.elf
	$(SIZE) $<
