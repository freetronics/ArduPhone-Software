#include <SPI.h>
#include <SD.h>

/*
 * Set the default baud rate of the GSM module from 115200 to 9600
 * - this enables the ArduPhone MCU running @ 8MHz to better handle its serial comms
 *
 * Normally only need to run this once if the GSM module has for any reason been re/set back
 * to factory firmware defaults.
 *
 * Use the serial monitor set at 9600 baud to monitor the output of these changes
 * - Serial  = USB serial UART
 * - Serial1 = GSM module serial UART
 */

// Function prototype
void OutputSerail1ToSerial( int waitTime );

// GSM module pins
int GsmReady    = 18;
int GsmOnKey    = 19;
int GsmNetOk    = 20;
int GsmDsrCts   = 21;
int GsmSense1v8 = 22;

// --------------------------

void setup() {
  // Setup in/outputs
  pinMode( GsmReady, INPUT );
  pinMode( GsmOnKey, OUTPUT );
  pinMode( GsmNetOk, INPUT );
  pinMode( GsmDsrCts, OUTPUT );
  pinMode( GsmSense1v8, INPUT );
  
  pinMode( GsmSense1v8, OUTPUT );
  digitalWrite( GsmSense1v8, HIGH ); // turn on internal pull-up resistor
 
  // Open USB serial 
  Serial.begin( 9600 );
  
  // Initially communicate at GSM module default rate
  Serial1.begin( 115200 );

  // Let user know we're alive
  Serial.println( "== ArduPhoneGSMSetBaudRate ==" );

  // Clear/output anything in Serial1
  OutputSerail1ToSerial( 1000 );

  // Turn on GSM module
  Serial.print( "Beginning GSM power on: " );
  PowerOnGsmModule();
  Serial.println( "READY" );

  // Output any start up messages from module & wait for module to settle
  OutputSerail1ToSerial( 5000 );
  
  // Turn on ECHO
  Serial.print( "Turn off echo: " );
  Serial1.println( "ATE0" );
  // Output any messages from module
  OutputSerail1ToSerial( 1000 );
  Serial.println( "DONE" );
  
  // Change baud rate
  Serial.print( "Changing baud rate to 9600: " );
  Serial1.println( "AT+IPR=9600" );
  // Output any messages from module
  OutputSerail1ToSerial( 1000 );
  Serial.println( "DONE" );
  
  // Should now be communicating with GSM module at 9600
  Serial1.begin( 9600 );
  
  // Write as default setting
  Serial.print( "Saving as default setting: " );
  Serial1.println( "AT&W" );
  // Output any messages from module
  OutputSerail1ToSerial( 1000 );
  Serial.println( "DONE" );

  // Check communicating successfully now
  Serial.print( "Sending 'AT' command, expect 'OK': " );
  Serial1.println( "AT" );
  // Output any messages from module
  OutputSerail1ToSerial( 2000 );
  Serial.println( "DONE" );

  Serial.println( "FINISHED" );
}

void loop() {
}

void OutputSerail1ToSerial ( int waitTime ) {
  const int buf_size = 128 ;
  char buffer[ buf_size ];
  byte index = 0;
  long startTime = millis() ;
  do {
    while ( Serial1.available() ) {
      char inByte = Serial1.read();
      if ( index < buf_size ) buffer[ index++ ] = inByte;
    }
  } while ( ( millis() - startTime ) < waitTime );
  buffer[ index ] = 0;
  if ( index > 0 ) {
    Serial.print ( "\nResponse -> \"" );
    Serial.print( buffer );
    Serial.println ( "\"" );
  }
}
  
void PowerOnGsmModule ()
{
  // Begin t1
  digitalWrite( GsmOnKey, HIGH );
  digitalWrite( GsmDsrCts, HIGH );
  delay( 200 );
  // Begin t2
  digitalWrite( GsmOnKey, LOW );
  digitalWrite( GsmDsrCts, HIGH );
  delay( 2000 );
  // Begin t3
  digitalWrite( GsmOnKey, HIGH );
  digitalWrite( GsmDsrCts, HIGH );
  delay( 2500 );
  // Begin t4
  digitalWrite( GsmOnKey, LOW );
  digitalWrite( GsmDsrCts, LOW );
  delay( 100 );
}


