/// -*- Mode:c++ -*-
/**
 * Test of matrix keypad on ArduPhone. Open the serial monitor at
 * 38400bps, then press keys on the keypad. Detected keys will be
 * shown in the serial monitor.
 *
 * Uses the Keypad library. Based on example by Alexander Brevig.
 */
#include <Keypad.h>

const byte ROWS = 6;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'!','U','?'},
  {'L','D','R'},
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {A6, A4, A3, A2, A1, A0};  // 25, 27, 28, 29, 30, 31
byte colPins[COLS] = {A5, A7, 23};              // 26, 24, 23

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(38400);
}

void loop(){
  char key = keypad.getKey();

  if (key){
    Serial.println(key);
  }
}
