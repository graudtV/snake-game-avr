MCU = atmega8535
CC = avr-gcc
OBJCOPY = avr-objcopy
F_CPU=1000000
EXTRA_FLAGS = -std=gnu99 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os

ifeq ($(TARGET),)
TARGET=main
endif

SRCS = $(TARGET).c
HEADERS_PATH = include

.PHONY: all build flash clean

all: build

build: $(TARGET).hex

flash: $(TARGET).hex
	avrdude -p $(MCU) -c usbasp -U flash:w:$(TARGET).hex:i -F -P usb -B12 -v

$(TARGET).hex: $(TARGET).bin
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).bin $(TARGET).hex

$(TARGET).bin: $(SRCS) $(HEADERS_PATH)/*
	$(CC) $(EXTRA_FLAGS) $(CFLAGS) -I $(HEADERS_PATH) -o $(TARGET).bin $(SRCS)

clean:
	rm -f *.bin *.hex
