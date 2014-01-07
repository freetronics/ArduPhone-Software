// -*- Mode:c++ -*-
/*
 * Freetronics ArduPhone - usbserial
 *
 * Functions related to the comms with the usb serial uart
 *
 * Cary Dreelan - Dec 2013
 */

// === Constants ===


// === States ===


// === Variables ===

boolean serialDebugMsgs = true ; // Output debug messages
boolean serialStatsMsgs = true ; // Output stats messages

// === Functions ===

// Send stats to USB serial UART (overloaded below)
void serialStatsOut ( const String sendBuffer ) {
  if ( serialStatsMsgs ) Serial.print( sendBuffer ) ;
}
void serialStatsOut ( const __FlashStringHelper* sendBuffer ) {
  if ( serialStatsMsgs ) Serial.print( sendBuffer ) ;
}
void serialStatsOut ( const unsigned long val ) {
  if ( serialStatsMsgs ) Serial.print( val ) ;
}

// Send debug messages to USB serial UART (overloaded below)
void serialDebugOut ( const String sendBuffer ) {
  if ( serialDebugMsgs ) Serial.print( sendBuffer ) ;
}
void serialDebugOut ( const __FlashStringHelper* sendBuffer ) {
  if ( serialDebugMsgs ) Serial.print( sendBuffer ) ;
}
void serialDebugOut ( const char val ) {
  if ( serialDebugMsgs ) Serial.print( val ) ;
}
void serialDebugOut ( const unsigned long val ) {
  if ( serialDebugMsgs ) Serial.print( val ) ;
}
void serialDebugOut ( const byte val ) {
  if ( serialDebugMsgs ) Serial.print( val ) ;
}

// =======================

void USBSerialSetup() {
  Serial.begin(38400);
}

// =======================

void USBSerialSlice() {
  // Could put things here related to a serial terminal menu implementation
}
