// -*- Mode:c++ -*-
/*
 * Freetronics ArduPhone - gsm
 *
 * Functions related to the GSM module
 *
 * Cary Dreelan - Dec 2013
 */

// === Constants ===

const byte GSM_PIN_READY     = 18 ;
const byte GSM_PIN_ON_KEY    = 19 ;
const byte GSM_PIN_NET_OK    = 20 ;
const byte GSM_PIN_DSR_CTS   = 21 ;
const byte GSM_PIN_SENSE_1V8 = 22 ;
const int  GSM_BAUD_RATE = 9600 ;
const byte GSM_SERIAL_BUF_SIZE = 128 ;

// === States ===
// State machine on this module for time slices
enum gsmStates {
  gsm_UNINITIALISED,
  gsm_SETUP,
  gsm_IDLE,
  gsm_ENQ_STATUS,
  gsm_POWER_OFF_SOFT,
  gsm_POWER_ON_SOFT
} ;
gsmStates gsmState ;
// State machine for display of GSM status in status bar
enum gsmDisplayStatusStates {
  gsmd_HARDWARE_OFF,
  gsmd_POWERING_ON_1,
  gsmd_POWERING_ON_2,
  gsmd_POWERING_ON_3,
  gsmd_POWERING_ON_4,
  gsmd_POWERING_ON_5,
  gsmd_UNKNOWN,
  gsmd_NORMAL, // Also shows signal strength
  gsmd_POWER_OFF_SOFT, // Equiv to airplane mode?
  gsmd_POWERING_ON_SOFT, // Transiioning from airplane to normal mode
  gsmd_REDRAW // Used to force if redraw required for last displayed status
} ;
gsmDisplayStatusStates gsmDisplayStatus, gsmLastDisplayedStatus ;
// State machine for gsm serial
enum gsmSerialStates {
  gsms_IDLE,
  gsms_ENQ_SIGNAL_STATUS,
  gsms_DIALLING
} ;
gsmSerialStates gsmSerialState ;

// === Variables ===

unsigned long nextGSMTime ;
byte gsmSignalStrength, gsmDisplayedSignalStrength ; // As per h/w module returned values are 0-31
String gsmSerialBuffer ;
byte gsmSerialBufferIndex = 0 ;
boolean gotOperatorName = false ;
String operatorName = "", operatorDisplayedName = "" ;
boolean gsmOKResponse = false ;

// === Functions ===

void PowerOnGsmModule ()
{
  if ( sliceStartTime >= nextGSMTime ) {
    switch ( gsmDisplayStatus ) {

      case gsmd_HARDWARE_OFF :
        digitalWrite( GSM_PIN_ON_KEY, HIGH ) ;
        digitalWrite( GSM_PIN_DSR_CTS, HIGH ) ;
        gsmDisplayStatus = gsmd_POWERING_ON_1 ;
        nextGSMTime = sliceStartTime + 200 ;
        break ;

      case gsmd_POWERING_ON_1 :
        digitalWrite( GSM_PIN_ON_KEY, LOW ) ;
        digitalWrite( GSM_PIN_DSR_CTS, HIGH ) ;
        gsmDisplayStatus = gsmd_POWERING_ON_2 ;
        nextGSMTime = sliceStartTime + 2000 ;
        break ;

      case gsmd_POWERING_ON_2 :
        digitalWrite( GSM_PIN_ON_KEY, HIGH ) ;
        digitalWrite( GSM_PIN_DSR_CTS, HIGH ) ;
        gsmDisplayStatus = gsmd_POWERING_ON_3 ;
        nextGSMTime = sliceStartTime + 2500 ;
        break ;

      case gsmd_POWERING_ON_3 :
        digitalWrite( GSM_PIN_ON_KEY, LOW ) ;
        digitalWrite( GSM_PIN_DSR_CTS, LOW ) ;
        gsmDisplayStatus = gsmd_POWERING_ON_4 ;
        nextGSMTime = sliceStartTime +  1500 ;
        gsmOKResponse = false ; // prep next state to check getting 'OK' responses
        break ;

      case gsmd_POWERING_ON_4 :
        // Should now be powered on
        // Wait for 'OK' response then initialise AT commands
        if ( gsmOKResponse ) {
          // GSM Modem has reposnded with OK
          // Send initialisation string(s) and pass onto next state
          SendGSMSerial( F("AT+COPS=1,0\r") ) ; // Return operator name in text rather than numerical format
          gsmDisplayStatus = gsmd_POWERING_ON_5 ;
        } else {
          // Try (again) to send 'AT' command to ensure gsm modem is responding before proceeding
          SendGSMSerial( F("AT\r") ) ;
          nextGSMTime = sliceStartTime + 1000 ; // Wait before next AT command attempt
        }
        break ;

      case gsmd_POWERING_ON_5 :
        // Should now be powered on, so go to unknown display status before polling
        gsmDisplayStatus = gsmd_UNKNOWN ;
        // Tell main state machine to start idling
        nextGSMTime = sliceStartTime + 500 ; // Wait before allowing IDLE to start making enquiries
        gsmState = gsm_ENQ_STATUS ; // To force enquiry before allowing other calls, etc
        break ;
    }
  }
}

void ProcessSignalStatus() {
  // Extract signal from buffer in form "+CSQ: X(X),YY"
  gsmSignalStrength = gsmSerialBuffer [ 6 ] - '0' ;
  if ( gsmSerialBuffer [ 7 ] != ',' ) {
    gsmSignalStrength *= 10 ;
    gsmSignalStrength += gsmSerialBuffer [ 7 ] - '0' ;
  }
  gsmDisplayStatus = gsmd_NORMAL ;
}

void ProcessOperatorName() {
  // Extract operator name from buffer in form '+COPS: 0,0,"XXXXXX"'
  byte count = 12 ;
  char nextChar ;
  operatorName = "" ;
  while ( ( nextChar = gsmSerialBuffer [ count ++ ] ) != '"' && nextChar != 0 ) {
    operatorName += nextChar ;
  }
  gotOperatorName = true ;
}

// Looks for chars from GSM module and fills buffer
void ReadGSMSerial() {
  while ( Serial1.available() ) {
    char inByte = Serial1.read() ;
    if ( inByte == 10 || inByte == 13 || gsmSerialBufferIndex == GSM_SERIAL_BUF_SIZE - 1 ) {
      // We have a end of line or full buffer, process it

      if ( gsmSerialBufferIndex > 0 ) {
        // DEBUG - BEGIN
        serialDebugOut ( F("ReadGSMSerial -> ") ) ;
        serialDebugOut ( gsmSerialBuffer ) ;
        serialDebugOut ( F("\n") ) ;
        // DEBUG - END

        switch ( gsmSerialState ) {

          case gsms_ENQ_SIGNAL_STATUS :
            // Expecting either a signal or operator response
            if ( gsmSerialBuffer.startsWith( "+CSQ:" ) ) {
              ProcessSignalStatus() ;
              gsmSerialState = gsms_IDLE ;
              gsmState = gsm_IDLE ;
            } else if ( gsmSerialBuffer.startsWith( "+COPS:" ) ) {
              ProcessOperatorName() ;
              gsmSerialState = gsms_IDLE ;
              gsmState = gsm_IDLE ;
            }
            break ;

          case gsms_DIALLING :
            ProcessMakeCallResponse() ;
            break ;

          case gsms_IDLE :
            // Drop through to default...
            ;
          default :
            // See if incoming RING message if not expecting anything
            ;
        }

        if ( gsmState == gsm_SETUP ) {
          if ( gsmSerialBuffer.startsWith( "OK" ) ) {
            gsmOKResponse = true ;
          }
        }

        // Clear the input buffer
        gsmSerialBuffer = "" ;
        gsmSerialBufferIndex = 0 ;
      }
    } else {
      // Add byte to buffer
      gsmSerialBuffer += inByte ;
      gsmSerialBufferIndex ++ ;
    }
  }
}

// Send chars to GSM module and clears receive buffer (overloaded below)
void SendGSMSerial( String sendBuffer ) {
  // First clear anything out of UART+buffer and discard
  while ( Serial1.available() ) Serial1.read() ;
  gsmSerialBuffer = "" ;
  gsmSerialBufferIndex = 0 ;
  // Send the passed in buffer
  Serial1.print( sendBuffer ) ;

  // DEBUG - BEGIN
  serialDebugOut ( F("SendGSMSerial -> ") ) ;
  serialDebugOut ( sendBuffer + "\n" ) ;
  // DEBUG - END
}

// Send chars to GSM module and clears receive buffer (overloaded above)
void SendGSMSerial( const __FlashStringHelper* sendBuffer ) {
  // First clear anything out of UART+buffer and discard
  while ( Serial1.available() ) Serial1.read() ;
  gsmSerialBuffer = "" ;
  gsmSerialBufferIndex = 0 ;
  // Send the passed in buffer
  Serial1.print( sendBuffer ) ;

  // DEBUG - BEGIN
  serialDebugOut ( F("SendGSMSerial F() -> ") ) ;
  serialDebugOut ( sendBuffer ) ;
  serialDebugOut ( F("\n") ) ;
  // DEBUG - END
}

// Send chars to GSM module and clears receive buffer (overloaded above)
void SendGSMSerial( char * sendBuffer ) {
  // First clear anything out of UART+buffer and discard
  while ( Serial1.available() ) Serial1.read() ;
  gsmSerialBuffer = "" ;
  gsmSerialBufferIndex = 0 ;
  // Send the passed in buffer
  Serial1.print( sendBuffer ) ;

  // DEBUG - BEGIN
  serialDebugOut ( F("SendGSMSerial char * () -> ") ) ;
  serialDebugOut ( sendBuffer ) ;
  serialDebugOut ( F("\n") ) ;
  // DEBUG - END
}

void EnquireGSMStatus() {
  if ( sliceStartTime >= nextGSMTime && ( gsmSerialState == gsms_IDLE || gsmSerialState == gsms_ENQ_SIGNAL_STATUS ) ) {

    if ( gotOperatorName ) {
      SendGSMSerial( F("AT+CSQ\r") ) ; // Get signal strength
    } else {
      SendGSMSerial( F("AT+COPS?\r") ) ; // Enquire operator name
    }
    gsmSerialState = gsms_ENQ_SIGNAL_STATUS ;

    // Check if really got operator name as may have enquired too soon (still locating)
    // Do the test here rather than above so that enquiry alternates between two until found
    if ( operatorName.length() == 0 ) gotOperatorName = false ;

    // Wait before requesting again
    nextGSMTime = sliceStartTime + 5000 ; // 5 seconds (GSM module doesn't seem to update signal strength faster than this)
  }
}

// =======================

void GsmSetup() {
  pinMode( GSM_PIN_READY, INPUT ) ;
  pinMode( GSM_PIN_ON_KEY, OUTPUT ) ;
  pinMode( GSM_PIN_NET_OK, INPUT ) ;
  pinMode( GSM_PIN_DSR_CTS, OUTPUT ) ;
  pinMode( GSM_PIN_SENSE_1V8, INPUT ) ;

  pinMode( GSM_PIN_SENSE_1V8, OUTPUT ) ;
  digitalWrite( GSM_PIN_SENSE_1V8, HIGH ) ; // turn on internal pull-up resistor

  Serial1.begin( GSM_BAUD_RATE ) ;

  // Initial state of this module
  gsmState = gsm_UNINITIALISED ;

  // Initial state of display status
  gsmDisplayStatus = gsmd_HARDWARE_OFF ;
  gsmLastDisplayedStatus = gsmd_REDRAW ;

  // Initial state of gsm serial
  gsmSerialState = gsms_IDLE ;

  // Buffer to receive chars
  gsmSerialBuffer.reserve( GSM_SERIAL_BUF_SIZE ) ;
}

// =======================

void GsmSlice() {
  switch ( gsmState ) {
    case gsm_UNINITIALISED :
      nextGSMTime = sliceStartTime + 1000 ; // Wait 1000 ms before attempting power up
      gsmState = gsm_SETUP ;
      gsmDisplayStatus = gsmd_HARDWARE_OFF ;
      break ;

    case gsm_SETUP :
      PowerOnGsmModule() ;
      ReadGSMSerial() ; // Checking AT command responses
      break ;

    case gsm_ENQ_STATUS :
      EnquireGSMStatus() ;
      ReadGSMSerial() ;
      gsmState = gsm_IDLE ;
      break ;

    case gsm_IDLE :
      EnquireGSMStatus() ;
      ReadGSMSerial() ;
      break ;

    case gsm_POWER_OFF_SOFT :

      break ;

    case gsm_POWER_ON_SOFT :

      break ;
  }
}
