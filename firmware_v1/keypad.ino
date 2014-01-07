// -*- Mode:c++ -*-
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
const unsigned KEYPAD_MAX_IDLE = 1000;

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
// https://en.wikipedia.org/wiki/Telephone_keypad
// https://en.wikipedia.org/wiki/E.161
/*

    0 = none (in some telephones, "OPERATOR" or "OPER")
    1 = none (in some older telephones, QZ)
    2 = ABC
    3 = DEF
    4 = GHI
    5 = JKL
    6 = MNO
    7 = PQRS (in some older telephones, PRS)
    8 = TUV
    9 = WXYZ (in some older telephones, WXY)

/// TODO: lower-case, symbols, move to PROGMEM
*/
char * keys2[ ] = {
  "1!@$%^&()",     "2ABC","3DEF",
  "4GHI",          "5JKL","6MNO",
  "7PQRS",         "8TUV","9WXYZ",
  "*-=_+[]{}\\|",  "0 ",  "#;':\",./<>?",
  0,
};

byte keypadRowPins[ KEYPAD_ROWS ] = { A6, A4, A3, A2, A1, A0 } ;  // 25, 27, 28, 29, 30, 31
byte keypadColPins[ KEYPAD_COLS ] = { A5, A7, 23 } ;              // 26, 24, 23
Keypad keypad = Keypad( makeKeymap( keys ), keypadRowPins, keypadColPins, KEYPAD_ROWS, KEYPAD_COLS ) ;
unsigned long lastCheckKeyTime ;

// === Functions ===

void checkKeyPressed() {
  if ( ( sliceStartTime - lastCheckKeyTime ) >= CHECK_KEY_PERIOD ) {
    char key = keypad.getKey() ;
    if ( key ) {
      // DEBUG - BEGIN
      serialDebugOut ( F("checkKeyPressed -> ") ) ;
      serialDebugOut ( key ) ;
      serialDebugOut ( F("\n") ) ;
      // DEBUG - END

      handleKeyPressed ( key ) ;
    }
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

char translate(char raw, bool & del)
{
  static long last = 0;
  bool old = (sliceStartTime - last) > KEYPAD_MAX_IDLE;
  last = sliceStartTime;

  // keep track if which key was pressed last and where in the list
  // of alternatives for that key we are
  static byte lastRaw =  '\0';
  static byte lastChr = -1;

  // did the user hit the same key as last time?
  bool same = (raw == lastRaw);
  lastRaw = raw;

  // delete iff we're translating, which only happens if the same key
  // is pressed within the timeout
  del = same && !old;

  // not translating/deleting? reset the character index
  // (i.e. "2ABC" starts by returning "A")
  if (!del)
  {
    lastChr = 0;
  }

  // need to translate...
  for (unsigned i = 0; keys2[i]; ++i)
  {
    // try to find the key in the translation table
    if (raw != keys2[i][0]) continue;
    // advance character index, wrapping if neded
    if (!keys2[i][++lastChr]) lastChr = 0;
    // return the translated character
    return keys2[i][lastChr];
  }
  // the translation table contains all the keys, so we should never get here.
  // keep this though, we can remove keys with a single translation option
  // later to save space.
  return raw;
}
