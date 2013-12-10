#!/bin/sh
/Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/avrdude \
		-C /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/etc/avrdude.conf \
		-p m1284p -F -P usb -c usbasp \
		-U flash:w:optiboot/optiboot_atmega1284p_8mhz.hex:a \
		-U lfuse:w:0xff:m -U hfuse:w:0xde:m -U efuse:w:0xfd:m -U unlock:w:0x3F:m -U lock:w:0x0F:m
say done
