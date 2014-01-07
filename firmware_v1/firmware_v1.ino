// -*- Mode:c++ -*-
/*
 * Freetronics ArduPhone - firmware_v1
 * - Initial firmware to support functions similar to a basic mobile phone.
 *
 * Requires
 * - Freetronics / FTOLED library from Github to support the 128x128 OLED display
 * - Arduino playroundd / Keypad library to support the keyboard matix
 *
 * Uses a co-operative multitasking approach where functions or modules never block
 * and rather use state machine(s) to track status and functions. Effectively each
 * main module of the system gets a time 'slice' in which it should perform any actions
 * as quickly as possible and then yield.
 *
 * Main file to incorporate all of the various functions of the system and track slice stats
 *
 * Cary Dreelan - Dec 2013
 */

// Includes used by any of the files should be inserted here for Arduino IDE to collate correctly
typedef char PROGMEM prog_char;
#include <FTOLED.h>
#include <FTOLED_Colours.h>
#include <progmem_compat.h>

#include <SPI.h>
#include <SD.h>
#include <Keypad.h>

#include <fonts/Droid_Sans_12.h>
#include <fonts/Arial_Black_16.h>

#include "ui.h"

// === Constants ===
// used for stats - first is power of 2 of second for division
const int sliceStatsPeriod = 4096 ; // in ms (2^12)
const byte sliceStatsShift = 12 ;

// Globals
unsigned long sliceStartTime = 0 ; // track start time of each time slice in ms
unsigned long sliceCount = 0 ; // how many slices since last stats were output
unsigned long sliceLastOutputTime = millis() ; // time in ms last stat about slices were output

void setup() {
  // Setup h/w and state machines, but don't power on
  GsmSetup() ;
  ScreenSetup() ;
  KeypadSetup() ;
  SoundSetup() ;
  USBSerialSetup() ;
  UISetup() ;
}

void loop() {
  // Track start time of this time slice in ms
  sliceStartTime = millis() ;

  // Give each module a slice of time
  GsmSlice() ;
  ScreenSlice() ;
  KeypadSlice() ;
  SoundSlice() ;
  USBSerialSlice() ;
  UISlice() ;

  // Gather/output some stats
  sliceCount ++ ;
  if ( ( sliceStartTime - sliceLastOutputTime ) >= sliceStatsPeriod ) {
    serialStatsOut( F("Slices p/sec: ") ) ;
    serialStatsOut( ( sliceCount * 1000 ) >> sliceStatsShift ) ;
    serialStatsOut( F("\n") ) ;

    // Reset for next iteration
    sliceLastOutputTime = sliceStartTime ;
    sliceCount = 0 ;
  }
}
