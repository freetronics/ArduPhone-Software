/*
 * Freetronics ArduPhone - keypad
 *
 * Functions related to the keypad - keys being pressed etc.
 *
 * Cary Dreelan - Dec 2013
 */


// === Constants ===

const byte KEYPAD_ROWS = 6 ;
const byte KEYPAD_COLS = 3 ;
const byte CHECK_KEY_PERIOD = 5 ; // in ms

// === States ===

enum keypadStates {
  KEYPAD_UNINITIALISED,
  KEYPAD_SETUP,
  KEYPAD_IDLE
} ;
keypadStates keypadState ;

// === Variables ===

char keys[ KEYPAD_ROWS ][ KEYPAD_COLS ] = {
  {'!','U','?'},
  {'L','D','R'},
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
} ;
byte keypadRowPins[ KEYPAD_ROWS ] = { A6, A4, A3, A2, A1, A0 } ;  // 25, 27, 28, 29, 30, 31
byte keypadColPins[ KEYPAD_COLS ] = { A5, A7, 23 } ;              // 26, 24, 23
Keypad keypad = Keypad( makeKeymap( keys ), keypadRowPins, keypadColPins, KEYPAD_ROWS, KEYPAD_COLS ) ;
unsigned long lastCheckKeyTime ;

// === Functions ===

void checkKeyPressed() {
  if ( ( sliceStartTime - lastCheckKeyTime ) > CHECK_KEY_PERIOD ) {
    char key = keypad.getKey() ;
    if ( key ) Serial.println( key ) ;
    lastCheckKeyTime = sliceStartTime ;
  }
}

// =======================

void KeypadSetup() {
  keypadState = KEYPAD_UNINITIALISED ;
}

// =======================

void KeypadSlice() {
  switch ( keypadState ) {
    case KEYPAD_UNINITIALISED :
      keypadState = KEYPAD_SETUP ;
      break ;
      
    case KEYPAD_SETUP :
      keypadState = KEYPAD_IDLE ;
      break ;
      
    case KEYPAD_IDLE :
      checkKeyPressed() ;
      break ;
  }
}
