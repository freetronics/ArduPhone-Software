firmware_v1
===========

This directory contains an initial version of firmware for the
Freetronics ArduPhone.

It is an attempt at creating the functions of a basic mobile
phone. This directory can be used as a starting point, as it will
unlikely complete all of the functions needed by a mobile/cell 
phone. That said, its intent is to try and replicate the functions
and user interface of a basic phone.

Requirements
------------

This code has been written using the Arduino 1.0.5 IDE. This was the
latest 1.0.X series available at the time of writing. The project
takes advantage of the multiple tabs and files that are possible for
an Arduino project and some of its peculiarities (eg. #includes in the
main file due to the way the IDE concatenates files.)

Like some of the examples, this firmware requires external libraries.
Please install these libraries before using this firmware.

 * FTOLED. Driver for the OLED128 display module fitted to
   ArduPhone. Download and instructions: 
   https://github.com/freetronics/FTOLED
 * Keypad. Detects and reports key presses on the matrix keypad.
   Download and instructions: http://playground.arduino.cc/code/Keypad

Approach
--------
As simple as a basic mobile phone is, there are many things that need
to be managed simultaneously. As well as handling keypad input and 
updating the screen, the GSM module can have multiple actions occurring
at the same time, eg. making a call while receiving an SMS. 

Therefore the approach has been to use a simplistic, yet effective
co-operative multi-tasking framework. Basically the multiple files
loosely represent 'modules' that manage each of the hardware components
or core functions of the phone. The idea is that the main 'firmware_v1'
file will call each 'module's' setup and then give it a time 'slice'
for each iteration of the main loop. Each module must then keep track
of its own state and timing of when things must occur. They should all
use the global 'sliceStartTime' to determine duration of events so that
multiple calls to millis() are not needed. Simply put, all functions
should therefore do an action as quickly as possible and then yield.

The screen layout uses a status bar across the top and a main frame for
the rest of the user interface to work in. An 'Activity' animation has
been created to quickly verify that the main loop is continuing to
be called and that blocking code or functions have not locked up the 
phone. As mentioned previously, events from the GSM module or other
parts of the phone can be missed if any blocking functions are called.
Therefore the delay function, large while or for loops or any library
functions that block should *NOT* be used. Rather, as is prevalent
throughout the code, use state machines and timing variables to
determine when something should next happen or something last happened.

Lastly, this code has been hopefully written to favour readability over
complexity or advanced functions. While the use of non-blocking code
requires state machines and timing, it should still be reasonably easy
to follow. Later code or size optimisations may mean this readability
will be compromised.

Features
--------
The following features/functions of the code or framework have been 
implemented:

 * Initial file structure (missing ui.ino for to do items though)
 * Multi-tasking and initial state machines
 * Status bar setup and drawing
 * Power on of GSM module and status bar reporting of each step
 * Activity animation showing main loop is running (top right of
   status bar)
 * USB Serial UART debug and time slice stats
 * Initial (incomplete) GSM enquiry and multi-tasking receive handling
 * GSM module status; from powering on to idle
 * Signal strength enquiry and graphing in status bar
 * Keypad setup (though presently not used until UI started)

To do
-----
In rough order, the following features/functions are still yet to be 
implemented:

 * Initial User Interface - basic menu and drawing and keypad handling
 * Screen saver - power off screen after set time of inactivity
 * Initiate call - typed number only
 * Send SMS - typed number only and T9(?) style key input
 * Basic sound alerts on buzzer - incoming call or SMS
 * Lock / unlock keypad
 * Receive call - display number
 * Receive / display SMS at time - no storage
 * Create phone book entry - save on SIM
 * Initiate call from phone book
 * Delete phone book entry - delete from SIM
 * Initiate SMS from phone book
 * Store / display received SMS
 * Airplane mode - turn GSM power on / off
 * Low power mode - put MCU/screen to sleep, but wake on key or GSM UART
 * Call list - show made, received, missed calls

It is not anticipated that all of these things can be implemented
before the Arduino Mini Conf 2014, but as many as possible in the time
available will be addressed.

Problems
--------

Some issues have been noted with both Serial (USB Serial UART) and 
Serial1 (GSM module UART) are used at the same time. It appears
they are interfering with each other and scrambling communications.
This has mainly been noted if using the USB serial UART for debug
around the same time as GSM module UART is receiving.

Initiating a connection the USB serial UART effectively resets the
phone. This has yet to be investigated as to whether this side effect
can be disabled.

NOTE
----
This is an early *ALPHA* release. While it should compile and run the 
listed features, it is not yet considered complete for the 2014 
Arduino Mini Conf. As more code is implemented, this document will be
updated and items in the to do list will be moved into the features
list.