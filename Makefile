
PLATFORM = AVR
MCU      = attiny13a
OSDIR    = MicroHAL/src
TARGET   = ny

INCLUDES =

SOURCES  = main.c hal.c

include $(OSDIR)/platform/build.mk

flash: build/$(TARGET)
#	avr-objdump -d build/$(TARGET)
	avr-size build/$(TARGET)
	avrdude -c avrftdi -b 57600 -p t13 -P /dev/ttyUSB0 -u -U $<
