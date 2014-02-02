ArduPhone Software
==================

This repository contains various supporting items for ArduPhone.

This README contains steps for setting up the ArduPhone-related
software on your computer.

Libraries
---------

Some of the examples require external libraries. Please install
these libraries before using the examples.

 * FTOLED. Driver for the OLED128 display module fitted to
   ArduPhone. Not required if you will not be using the display.
   Download and instructions: https://github.com/freetronics/FTOLED
 * Keypad. Detects and reports keypresses on the matrix keypad.
   Download and instructions: http://playground.arduino.cc/code/Keypad

## Installing the Libraries

For a guide to installing Arduino Libraries, see this page:
http://www.freetronics.com/pages/how-to-install-arduino-libraries

FTDI Drivers
------------

On OS X and Windows you will need a driver for the FTDI USB to Serial
interface chip on the ArduPhone.

Recent versions of Windows may automatically install the drivers via
Windows Update. If they don't automatically install then you'll need
to download them and manually run the installation.

Download link for OS X and Windows is here:
http://www.ftdichip.com/Drivers/VCP.htm

There is a link on that page to
[FTDI Installation Guides](http://www.ftdichip.com/Support/Documents/InstallGuides.htm)
for various OSes.

Linux users don't need any extra drivers (yay Linux!)

Board Profile
-------------

The board profile allows the Arduino IDE to recognise the ArduPhone
hardware and correctly compile and upload new sketches to it. The
profile defines the MCU speed and type, and associates specific I/O
pins to pin names so you can use labels such as "A3" to address
analog input 3, for example.

## Installing the board profile

Locate the "sketchbook" directory where your Arduino sketches are
stored, and check inside it for a directory called "hardware" (this is
alongside the "libraries" directory as well.)

If the "hardware" directory doesn't exist, create it. Place the
directory called "ArduPhone" (located inside "BoardProfile" here) into
the "hardware" directory and restart the Arduino IDE.

The directory hierarchy should be:

    sketchbook directory -> hardware -> ArduPhone

From inside the IDE, select "Tools > Board > Freetronics ArduPhone".
You can now compile and upload new sketches to your ArduPhone.

Examples
--------
The "examples" directory contains various sketches that you can use
to experiment with your ArduPhone.

To make the sketches available, copy them to your sketchbook 

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

firmware_v1
-----------
This folder contains the firmware that allows you to use an ArduPhone as a
basic mobile/cell phone. It also shows how most of the hardware is used
to combine the various examples into a working phone.
 