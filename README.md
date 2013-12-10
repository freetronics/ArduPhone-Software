ArduPhone Software
==================

This repository contains various supporting items for ArduPhone.

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
