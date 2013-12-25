ArduPhone Software
==================

This repository contains various supporting items for ArduPhone.

Some of the examples require external libraries. Please install
these libraries before using the examples.

 * FTOLED. Driver for the OLED128 display module fitted to
   ArduPhone. Not required if you will not be using the display.
   Download and instructions: https://github.com/freetronics/FTOLED
 * Keypad. Detects and reports keypresses on the matrix keypad.
   Download and instructions: http://playground.arduino.cc/code/Keypad

Board Profile
-------------
The board profile allows the Arduino IDE to recognise the ArduPhone
hardware and correctly compile and upload new sketches to it. The
profile defines the MCU speed and type, and associates specific I/O
pins to pin names so you can use labels such as "A3" to address
analog input 3, for example.

Locate the "sketchbook" directory where your Arduino sketches are
stored, and check inside it for a directory called "hardware". If it
doesn't exist, create it. Place the directory called "ArduPhone"
(located inside "BoardProfile") into the "hardware" directory and
restart the Arduino IDE.

From inside the IDE, select "Tools > Board > Freetronics ArduPhone".
You can now compile and upload new sketches to your ArduPhone.

Examples
--------
The "examples" directory contains various sketches that you can use
to experiment with your ArduPhone.

Some of the examples require external libraries, as linked above.

 * ArduPhoneButtonTest: Detects keypresses on the matrix keypad and
   reports them via the serial monitor.
 * ArduPhoneOLEDCountdown: Test of the OLED screen.
 * ArduPhoneSerialProxy: Opens a serial connection to both the
   ADH8066 GSM module and to the host PC via USB. Anything typed
   into the serial monitor will be relayed to the GSM module, and
   vice versa.
 * ArduPhoneSerialProxyOLED: Same as the ArduPhoneSerialProxy example,
   but also displays messages sent and received on the OLED display.
 * ArduPhoneSetGSMBaudRate: Used to set the default baud rate the GSM 
   module communicates with. 