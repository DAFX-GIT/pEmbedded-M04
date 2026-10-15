MAKEFLAGS += --no-print-directory
F_CPU = 16000000UL
SOURCES=main.c
BUILD_DIR = build
PROGRAM_NAME = woaw
BIN = $(BUILD_DIR)/$(PROGRAM_NAME).bin
HEX = $(BUILD_DIR)/$(PROGRAM_NAME).hex

all: bfc

hex: $(HEX)


$(HEX): $(BIN)
	avr-objcopy -j .text -j .data -O ihex $< $@

$(BIN): $(SOURCES) | $(BUILD_DIR)
	avr-gcc -mmcu=atmega328p -DF_CPU=$(F_CPU) -Wall -Os -o $@ $(SOURCES)
	
$(BUILD_DIR):
	mkdir -pv $(BUILD_DIR)


flash: hex
	avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:$(HEX)	

clean: 
	rm -rf $(BUILD_DIR)

bfc:
	$(MAKE) hex
	$(MAKE) flash
	$(MAKE) clean

.PHONY: all hex flash clean bfc