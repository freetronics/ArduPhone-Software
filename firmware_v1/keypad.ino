// -*- Mode:c++ -*-
/*
 * Freetronics ArduPhone - keypad
 *
 * Functions related to the keypad - keys being pressed etc.
 *
 * Cary Dreelan - Dec 2013
 * Thomas Sprinkmeier - Jan 2014 (SMS functionality)
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
  The letters are defined.
  Other stuff is not (symbols, punctuation etc.)
  Modify the xlate_* strings below to alter the translation table
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
*/
/// TODO: lower-case
/// store in PROGMEM to save precious RAM
/// http://www.arduino.cc/en/Reference/PROGMEM
/// These strings provide alternate characters
/// "<character on keypad><alt1><alt2><alt3>..."
const char xlate_0[] PROGMEM = "0 ";
const char xlate_1[] PROGMEM = "1!@$%^&()";
const char xlate_2[] PROGMEM = "2ABC";
const char xlate_3[] PROGMEM = "3DEF";
const char xlate_4[] PROGMEM = "4GHI";
const char xlate_5[] PROGMEM = "5JKL";
const char xlate_6[] PROGMEM = "6MNO";
const char xlate_7[] PROGMEM = "7PQRS";
const char xlate_8[] PROGMEM = "8TUV";
const char xlate_9[] PROGMEM = "9WXYZ";
const char xlate_S[] PROGMEM = "*-=_+[]{}\\|"; // Star
const char xlate_H[] PROGMEM = "#;':\",./<>?"; // Hash
// make sure you adjust this if you modify the xlate strings!
#define MAX_XLATE_LENGTH  12
// list of translation strings
PROGMEM const char * const xlate[] =
{
    xlate_1,    xlate_2,    xlate_3,
    xlate_4,    xlate_5,    xlate_6,
    xlate_7,    xlate_8,    xlate_9,
    xlate_S,    xlate_0,    xlate_H,
};
const byte NUM_XLATE_STRINGS =
    sizeof(xlate)/
    sizeof(*xlate);

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

  // use the lower-case version of the symbol
  static bool useLower = false;

  // not translating/deleting? reset the character index
  // (i.e. "2ABC" starts by returning "A")
  if (!del)
  {
    lastChr  = 0;
    useLower = false;
  }

  // need to translate...
  for (unsigned i = 0; i < NUM_XLATE_STRINGS; ++i)
  {
      // move the translation string to RAM to make it easier to use...
      char buff[MAX_XLATE_LENGTH+1];
      strcpy_P(buff, (char*)pgm_read_word(&(xlate[i])));

      // try to find the key in the translation table
      if (raw != *buff) continue;
      // advance character index, wrapping if neded
      if (!buff[++lastChr])
      {
          lastChr = 0;
          // on wrap-around toggle use-lower flag
          useLower = !useLower;
      }
      // return the translated character
      return useLower ? tolower(buff[lastChr]) : buff[lastChr];
  }
  // the key isn't in the translation table, don't translate it.
  return raw;
}
