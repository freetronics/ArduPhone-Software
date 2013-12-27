/*
 * Freetronics ArduPhone - keypad
 *
 * Functions related to the keypad - keys being pressed etc.
 *
 * Cary Dreelan - Dec 2013
 */


// === Constants ===

const byte keypad_ROWS = 6 ;
const byte keypad_COLS = 3 ;
const byte checkKeyPeriod = 5 ; // in ms

// === States ===

enum keypadStates {
  keypad_UNINITIALISED,
  keypad_SETUP,
  keypad_IDLE
} ;
keypadStates keypadState ;

// === Variables ===

char keys[ keypad_ROWS ][ keypad_COLS ] = {
  {'!','U','?'},
  {'L','D','R'},
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
} ;
byte keypadRowPins[ keypad_ROWS ] = { A6, A4, A3, A2, A1, A0 } ;  // 25, 27, 28, 29, 30, 31
byte keypadColPins[ keypad_COLS ] = { A5, A7, 23 } ;              // 26, 24, 23
Keypad keypad = Keypad( makeKeymap( keys ), keypadRowPins, keypadColPins, keypad_ROWS, keypad_COLS ) ;
unsigned long lastCheckKeyTime ;

// === Functions ===

void checkKeyPressed() {
  if ( ( sliceStartTime - lastCheckKeyTime ) > checkKeyPeriod ) {
    char key = keypad.getKey() ;
    if ( key ) Serial.println( key ) ;
    lastCheckKeyTime = sliceStartTime ;
  }
}

// =======================

void KeypadSetup() {
  keypadState = keypad_UNINITIALISED ;
}

// =======================

void KeypadSlice() {
  switch ( keypadState ) {
    case keypad_UNINITIALISED :
      keypadState = keypad_SETUP ;
      break ;
      
    case keypad_SETUP :
      keypadState = keypad_IDLE ;
      break ;
      
    case keypad_IDLE :
      checkKeyPressed() ;
      break ;
  }
}
