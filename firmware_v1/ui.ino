// -*- Mode:c++ -*-
/*
 * Freetronics ArduPhone - ui
 *
 * Functions related to running the user interface.
 *
 * Cary Dreelan - Dec 2013
 * Thomas Sprinkmeier - Jan 2014 (SMS functionality)
 */


// === Constants ===
// Startup animation values
const unsigned int STARTUP_FRAME_PERIOD = 55 ; // How long between each animation frame
const unsigned int STARTUP_HOLD_DELAY = 1500 ; // How long to wait after animation completes
const byte STARTUP_NUM_FRAMES = 38 ;
const byte STARTUP_TEXT_HEIGHT = 12 ;
const Colour STARTUP_BG_COLOUR = { 0, 0, 2 } ;
const Colour STARTUP_FG_COLOUR = YELLOW ;
// Main menu values
const Colour MAIN_MENU_BG_COLOUR = NAVY ;
const Colour MAIN_MENU_FG_COLOUR = WHITE ;
const byte MAIN_MENU_MAX_LINES = 7 ;
const byte MAIN_MENU_LINE_HEIGHT = 16 ;
const byte MENU_ITEM_TEXT_BUF_LEN = 32 ;
// Make call values
const Colour MAKE_CALL_BG_COLOUR = YELLOW ;
const Colour MAKE_CALL_FG_COLOUR = BLUE ;
const Colour MAKE_CALL_NUM_COLOUR = RED ;
const Colour MAKE_CALL_KEY_COLOUR = DARKGREEN ;
const byte PHONE_NUM_BUF_MIN_Y = MF_MIN_Y + 50 ;
const byte PHONE_NUM_BUF_SIZE = 14 ;
const unsigned char SMS_BUF_SIZE = 161 ;
const unsigned int MAKE_CALL_ERROR_DISPLAY_TIME = 3000 ; // How long to display error when dialilng
// Receive call values
const Colour RCV_CALL_BG_COLOUR = GREEN ;
const Colour RCV_CALL_FG_COLOUR = YELLOW ;
const Colour RCV_CALL_NUM_COLOUR = RED ;
const Colour RCV_CALL_KEY_COLOUR = DARKGREEN ;
const unsigned int RCV_CALL_HANG_UP_DISPLAY_TIME = 3000 ; // How long to display when hung up
// Send SMS values
const Colour SEND_SMS_FG_COLOUR  = YELLOW;
const Colour SEND_SMS_BG_COLOUR  = GREEN;
const Colour SEND_SMS_KEY_COLOUR = DARKGREEN;
const unsigned int SEND_SMS_DISPLAY_RESULT_DELAY = 2000 ; // How long to wait after displaying result


// === States ===

// uiStates enum moved to ui.h due to Arduino IDE limitation when enum used as function parameter
uiStates uiState, returnNumState ;
enum receiveCallStates {
  RC_WAITING_FOR_ACCEPT,
  RC_WAITING_FOR_MODEM,
  RC_ON_CALL,
  RC_HUNG_UP
} ;
receiveCallStates receiveCallState ;
enum makeCallStates {
    /// MakeCall states
  MC_DRAW_CALLING_NUMBER,
  MC_WAITING_FOR_MODEM,
  MC_DIALLING,
  MC_MODEM_ERROR,
  MC_ON_CALL,
  MC_HANG_UP,
} ;
makeCallStates makeCallState ;

enum makeSMSStates {
    /// SMS states
    /// set SMS text mode
    SMS_SEND_TEXT_MODE,
    /// wait for modem to acknowlege text mode
    SMS_WAIT_TEXT_MODE_OK,
    /// wait for ">" prompt to send SMS body
    SMS_WAIT_GT,
    /// wait for SMS delivery notification
    SMS_WAIT_OK,
    /// Display outcome
    SMS_DISPLAY_RESULT
} ;
makeSMSStates makeSMSState ;


// === Variables ===

unsigned long nextUITime ;
byte startupFrameNum = 0 ;
// Main menu variables - these next 3 items and their arrays must be in sync
const char mainMenu0[] PROGMEM = "Call";
const char mainMenu1[] PROGMEM = "SMS";
const char mainMenu2[] PROGMEM = "Lock keys";
const char mainMenu3[] PROGMEM = "Four";
const char mainMenu4[] PROGMEM = "Five";
const char mainMenu5[] PROGMEM = "Six";
const char mainMenu6[] PROGMEM = "Seven";
const char mainMenu7[] PROGMEM = "Eight";
const char mainMenu8[] PROGMEM = "Nine";
const char mainMenu9[] PROGMEM = "Ten";
PROGMEM const char * const main_menu_table[] = {
  mainMenu0,
  mainMenu1,
  mainMenu2,
  mainMenu3,
  mainMenu4,
  mainMenu5,
  mainMenu6,
  mainMenu7,
  mainMenu8,
  mainMenu9
} ;
const byte MAIN_MENU_NUM_ITEMS =
    sizeof(main_menu_table)/
    sizeof(*main_menu_table);
typedef void ( * FunctionPointer ) ();
FunctionPointer main_menu_functions[MAIN_MENU_NUM_ITEMS] = {
  makeCallMenuItem,
  smsMenuItem,
  lockKeysMenuItem,
  0,
  0,
  0,
  0,
  0,
  0,
  0
} ;

byte curMenuItem = 0, lastMenuItem = 0 ;
char phoneNumBuffer[ PHONE_NUM_BUF_SIZE ] ;
String callFromNumber ;
boolean gotCallFromNumber = false ;
char smsBuffer[ SMS_BUF_SIZE ];
byte phoneNumBufferIndex ;
unsigned char smsBufferIndex;
boolean phoneNumberEntered = false ;
boolean smsEntered = false ;

// === Functions ===

// Incoming Call related functions

void DrawIncomingCallNumber() {
  oled.selectFont( Arial_Black_16 ) ;
  oled.drawFilledBox( MF_MIN_X, PHONE_NUM_BUF_MIN_Y, MF_MAX_X, PHONE_NUM_BUF_MIN_Y + 16, RCV_CALL_BG_COLOUR ) ;
  byte posX = ( MF_MAX_X - oledStringWidth( & callFromNumber [ 0 ] ) ) / 2 ;
  oled.drawString( posX, PHONE_NUM_BUF_MIN_Y, callFromNumber, RCV_CALL_NUM_COLOUR, RCV_CALL_BG_COLOUR ) ;
}

void ProcessIncomingCall() {
  uiState = UI_RCV_CALL ;
  callFromNumber = "Unknown" ;
  gotCallFromNumber = false ;

  // Process gsm buffer string to get incoming number
  // TODO - need AT init commands in gsm setup to present caller ID
  // TODO - need to extract caller ID from 'RING' message in gsmSerialBuffer

  if ( screenState == screen_POWER_OFF ) {
    TurnDisplayOn() ;
  }
  // Draw that incoming call is happening
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, RCV_CALL_BG_COLOUR ) ;
  DrawIncomingCallNumber() ; // Also sets oled.selectFont
  oled.drawString( 26, MF_MAX_Y - 30, F("Call from"), RCV_CALL_FG_COLOUR, RCV_CALL_BG_COLOUR ) ;
  oled.drawString( 1, MF_MIN_Y, F("Answer"), RCV_CALL_KEY_COLOUR, RCV_CALL_BG_COLOUR ) ;
  oled.drawString( 70, MF_MIN_Y, F("Hangup"), RCV_CALL_KEY_COLOUR, RCV_CALL_BG_COLOUR ) ;
  // Initial state for making call
  receiveCallState = RC_WAITING_FOR_ACCEPT ;
}

void handleReceiveCall() {
  switch ( receiveCallState ) {

    case RC_WAITING_FOR_MODEM :
      // modem is now considered IDLE, answer the phone
      SendGSMSerial( F("ATA\r") ) ;
      receiveCallState = RC_ON_CALL ;
      oled.selectFont( Arial_Black_16 ) ;
      oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, RCV_CALL_BG_COLOUR ) ;
      oled.drawString( 70, MF_MIN_Y, F("Hang up"), MAKE_CALL_KEY_COLOUR, MAKE_CALL_BG_COLOUR ) ;
      break ;

    case RC_ON_CALL :
      // Do things like update time on call etc
      // Handle key press will deal with hang up
      break ;

    case RC_HUNG_UP :
      // Message has been displayed about hang up, wait and then go back to main menu
      if ( sliceStartTime >= nextUITime ) {
        uiState = UI_DRAW_MAIN_MENU ;
      }
      break ;

    // Else don't do anything for other states
  }
}

void displayHungUp() {
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, RCV_CALL_BG_COLOUR ) ;
  oled.selectFont( Arial_Black_16 ) ;
  oled.drawString( 13, MF_MAX_Y - 30, F("Hung up on"), RCV_CALL_FG_COLOUR, RCV_CALL_BG_COLOUR ) ;
  byte posX = ( MF_MAX_X - oledStringWidth( & callFromNumber [ 0 ] ) ) / 2 ;
  oled.drawString( posX, PHONE_NUM_BUF_MIN_Y, callFromNumber, RCV_CALL_NUM_COLOUR, RCV_CALL_BG_COLOUR ) ;
  nextUITime = sliceStartTime + RCV_CALL_HANG_UP_DISPLAY_TIME ;
  receiveCallState = RC_HUNG_UP ;
}

void ProcessReceiveCallHungUp() {
  // Other party has hung up
  displayHungUp() ;
}

void ProcessReceiveCallNumber() {
  if ( ! gotCallFromNumber ) {
    // Extract phone number that is calling us. Example response -> +CLIP: "0123456789",129,,,,0
    byte idx = 8 ;
    char nextChar ;
    String receiveCallNumber = "" ;
    while ( ( nextChar = gsmSerialBuffer [ idx ++ ] ) != '"' && nextChar != 0 ) {
      receiveCallNumber += nextChar ;
    }
    if ( receiveCallNumber.length() > 0 ) {
      callFromNumber = receiveCallNumber ;
      DrawIncomingCallNumber() ;
    }
    gotCallFromNumber = true ;
  }
}

// Make Call related functions
Colour drawPhoneBgColour = MAKE_CALL_NUM_COLOUR;
void drawPhoneNumberBuffer() {
  // Draw centered phone number with space before and after (needed incase of backspace)
  oled.selectFont( Arial_Black_16 ) ;
  char displayNumber[ PHONE_NUM_BUF_SIZE + 3 ] ; // To allow for a ' ' before and after
  displayNumber[ 0 ] = ' ' ;
  strncpy( displayNumber + 1, phoneNumBuffer, PHONE_NUM_BUF_SIZE ) ;
  displayNumber[ phoneNumBufferIndex + 1 ] = ' ' ;
  displayNumber[ phoneNumBufferIndex + 2 ] = '\0' ;
  byte posX = ( MF_MAX_X - oledStringWidth( displayNumber ) ) / 2 ;
  oled.drawString( posX, PHONE_NUM_BUF_MIN_Y, displayNumber, MAKE_CALL_NUM_COLOUR, drawPhoneBgColour) ;
}

void setupGetPhoneNumer( const uiStates returnState,
    const Colour & bgColour) {
  phoneNumberEntered = false ;
  phoneNumBuffer[ 0 ] = '\0' ;
  phoneNumBufferIndex = 0 ;
  returnNumState = returnState ;
  uiState = UI_GET_PHONE_NUM ;
  drawPhoneBgColour = bgColour;
}

void drawSMSBuffer ()
{
    smsBuffer[smsBufferIndex] = '\0';
    ScreenPrint(smsBuffer);
}

void setupGetSMS( const uiStates returnState )
{
    smsEntered     = false;
    smsBuffer[0]   = '\0';
    smsBufferIndex = 0;
    returnNumState = returnState ;
    uiState        = UI_SEND_SMS;
}

void makeCallMenuItem() {
  uiState = UI_MAKE_CALL ;
  // Setup screen to enter number
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, MAKE_CALL_BG_COLOUR ) ;
  oled.selectFont( Arial_Black_16 ) ;
  oled.drawString( 7, MF_MAX_Y - 30, F("Enter number"), MAKE_CALL_FG_COLOUR, MAKE_CALL_BG_COLOUR ) ;
  oled.drawString( 1, MF_MIN_Y, F("Delete"), MAKE_CALL_KEY_COLOUR, MAKE_CALL_BG_COLOUR ) ;
  oled.drawString( 50, MF_MIN_Y + 18, F("Call"), MAKE_CALL_KEY_COLOUR, MAKE_CALL_BG_COLOUR ) ;
  oled.drawString( 73, MF_MIN_Y, F("Cancel"), MAKE_CALL_KEY_COLOUR, MAKE_CALL_BG_COLOUR ) ;
  // Setup to get a phone number and return to UI_MAKE_CALL when complete/cancelled
  setupGetPhoneNumer( UI_MAKE_CALL, MAKE_CALL_BG_COLOUR ) ;
  // Initial state for making call
  makeCallState = MC_DRAW_CALLING_NUMBER ;
}

void handleMakeCall() {
  if ( phoneNumberEntered && phoneNumBufferIndex > 0 ) {
    switch ( makeCallState ) {

      case MC_DRAW_CALLING_NUMBER :
        oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, MAKE_CALL_BG_COLOUR ) ;
        oled.selectFont( Arial_Black_16 ) ;
        oled.drawString( 38, MF_MAX_Y - 30, F("Calling"), MAKE_CALL_FG_COLOUR, MAKE_CALL_BG_COLOUR ) ;
        drawPhoneNumberBuffer() ;
        makeCallState = MC_WAITING_FOR_MODEM ;
        break ;

      case MC_WAITING_FOR_MODEM :
        if ( gsmSerialState == gsms_IDLE && gsmState == gsm_IDLE ) {
          // modem is now considered IDLE, proceed to dial and set states
          SendGSMSerial( F("ATD") ) ;
          SendGSMSerial( phoneNumBuffer ) ;
          SendGSMSerial( F(";\r") ) ; // ';' needed for voice call
          makeCallState = MC_DIALLING ;
          gsmSerialState = gsms_DIALLING ;
          oled.drawString( 56, MF_MIN_Y, F("Hang up"), MAKE_CALL_KEY_COLOUR, MAKE_CALL_BG_COLOUR ) ;
        }
        break ;

      case MC_DIALLING :
        // Do nothing as ReadGSMSerial will call ProcessMakeCallResponse
        break ;

      case MC_MODEM_ERROR :
        if ( sliceStartTime >= nextUITime ) {
          // Error has been displayed for set time, return to main menu
          uiState = UI_DRAW_MAIN_MENU ;
        }
        break ;

      case MC_ON_CALL :
        break ;

      case MC_HANG_UP :
        // Hang up and return to main men
        SendGSMSerial( F("ATH\r") ) ;
        uiState = UI_DRAW_MAIN_MENU ;
        gsmState = gsm_IDLE ; // Ignore any response from hang up message
        // Important - must return serial state to idling for polling
        gsmSerialState = gsms_IDLE ;
        break ;
    }
  } else {
    // No number to call, return to main menu
    uiState = UI_DRAW_MAIN_MENU ;
  }
}

void ProcessMakeCallResponse() {
  if ( gsmSerialBuffer.startsWith( "OK" ) ) {
    makeCallState = MC_ON_CALL ;
  } else {
    if ( makeCallState != MC_HANG_UP ) {
      // Anything is else is a failure, let user know message (could be busy, etc.)
      oled.selectFont( Arial_Black_16 ) ;
      // Center the return message
      byte posX = ( MF_MAX_X / 2 ) - ( oledStringWidth( & gsmSerialBuffer [ 0 ] ) / 2 ) ;
      oled.drawString( posX, MF_MIN_Y + 20, gsmSerialBuffer, MAKE_CALL_NUM_COLOUR, MAKE_CALL_BG_COLOUR ) ;
      // Setup to wait for error to be visible for a time
      makeCallState = MC_MODEM_ERROR ;
      nextUITime = sliceStartTime + MAKE_CALL_ERROR_DISPLAY_TIME ;
    }
  // Important - must return serial state to idling for polling
  gsmSerialState = gsms_IDLE ;
  }
}


// SMS related functions

void smsMenuItem() {
    uiState = UI_CREATE_SMS ;
    // Temp display message to show function was selected
    nextUITime = sliceStartTime + 1500 ;
    oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, SEND_SMS_BG_COLOUR ) ;
    oled.selectFont( Arial_Black_16 ) ;

    oled.drawString( 7, MF_MAX_Y - 30, F("Enter number"), SEND_SMS_FG_COLOUR,  SEND_SMS_BG_COLOUR ) ;
    oled.drawString( 1, MF_MIN_Y, F("Delete"),            SEND_SMS_KEY_COLOUR, SEND_SMS_BG_COLOUR ) ;
    oled.drawString( 50, MF_MIN_Y + 18, F("Next"),        SEND_SMS_KEY_COLOUR, SEND_SMS_BG_COLOUR ) ;
    oled.drawString( 73, MF_MIN_Y, F("Cancel"),           SEND_SMS_KEY_COLOUR, SEND_SMS_BG_COLOUR ) ;
    // Setup to get a phone number and return to UI_MAKE_CALL when complete/cancelled
    setupGetPhoneNumer( UI_SEND_SMS, SEND_SMS_BG_COLOUR);
    // First state in SMS delivery state machine (after phone/text entry)
    makeSMSState = SMS_SEND_TEXT_MODE;
}

void handleSendSMS()
{
    // if the phone number hasn't been entered yet
    // then there's nothiung to do.
    if (!phoneNumberEntered) return;

    // state variable to avoid refresh/flicker of prompt
    static bool wasEmpty = false;
    // we have a phone number. prompt the user to enter a message
    if (!smsBufferIndex)
    {
        setupGetSMS(UI_SEND_SMS);
        oled.selectFont( Arial_Black_16 ) ;
        oled.drawString( 50, MF_MIN_Y + 18, F("Send"),
                         SEND_SMS_KEY_COLOUR, SEND_SMS_BG_COLOUR ) ;
        if (!wasEmpty)
        {
            ScreenPrint("Please enter your message ...\n");
        }
    }
    wasEmpty = !smsBufferIndex;
    // did the SMS get entered?
    if (!smsEntered) return;

    /// SMS delivery state machine
    switch (makeSMSState)
    {
        // make sure the modem is in TEXT mode
    case SMS_SEND_TEXT_MODE:
    {
        // Tell user what's happening and clear the button menu
        ScreenPrint("Sending message...\n");
        oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MIN_Y + 36, SEND_SMS_BG_COLOUR ) ;

        serialDebugOut (F("Selecting TEXT mode...\n"));
        SendGSMSerial( F("AT+CMGF=1\r"));
        makeSMSState    = SMS_WAIT_TEXT_MODE_OK;
        // disable the periodic query to avoid aborting the SMS
        gsmState        = gsm_WAIT;
        gsmSerialState  = gsms_IDLE;
    } break ;

    // wait for the modem to acknowlege TEXT mode
    // then enter the phone number
    case SMS_WAIT_TEXT_MODE_OK:
    {
        if ( gsmSerialBuffer.startsWith( "OK" ) )
        {
            serialDebugOut (F("Sending Phone number...\n"));
            SendGSMSerial( F("AT + CMGS=\""));
            // from this number
            SendGSMSerial( phoneNumBuffer );
            SendGSMSerial( F("\"\r"));
            gsmState        = gsm_WAIT;
            gsmSerialState  = gsms_IDLE;
            makeSMSState    = SMS_WAIT_GT;
            gsmSerialBuffer = "";
        }
    } break;

    // wait for the ">" prompt indicating the modem is ready for the SMS body
    case SMS_WAIT_GT:
    {
        if ( gsmSerialBuffer.startsWith( ">" ) )
        {
            serialDebugOut (F("Sending SMS body...\n"));
            SendGSMSerial( smsBuffer );
            // CNTRL-Z
            char junk[2];
            junk[0] = 26;
            junk[1] = 0;
            SendGSMSerial(junk);
            gsmState        = gsm_WAIT;
            gsmSerialState  = gsms_IDLE;
            gsmSerialBuffer = "";
            makeSMSState    = SMS_WAIT_OK;
        }
    } break;

    // wait for delivery notification
    case SMS_WAIT_OK:
    {
        if ( gsmSerialBuffer.startsWith( "OK" ) )
        {
            serialDebugOut (F("SMS Sent OK!...\n"));
            // Tell user message was sent
            ScreenPrint("Message sent\n");
            // Wait for message to be displayed
            nextUITime = sliceStartTime + SEND_SMS_DISPLAY_RESULT_DELAY ; // Wait while message displayed
            makeSMSState    = SMS_DISPLAY_RESULT ;
        }
        else if ( gsmSerialBuffer.startsWith( "ERROR" ) )
        {
            serialDebugOut (F("Unable to send SMS!\n"));
            // Tell user message was NOT sent
            ScreenPrint("Unable to send SMS!\n");

            // now what? try again? store the SMS on the SIM?

            // Wait for message to be displayed
            nextUITime = sliceStartTime + SEND_SMS_DISPLAY_RESULT_DELAY ; // Wait while message displayed
            makeSMSState    = SMS_DISPLAY_RESULT ;
        }
    } break;
    
    // Wait while user result of sending sms
    case SMS_DISPLAY_RESULT :
    {
      if ( sliceStartTime >= nextUITime ) 
      {
        // Finished displaying message, return to main menu and reset states
        uiState         = UI_DRAW_MAIN_MENU;
        gsmState        = gsm_IDLE;
        smsBufferIndex  = 0;
      }
    } break ;
    
    }
}

void lockKeysMenuItem() {
  uiState = UI_LOCK_KEYS ;
  // Temp display message to show function was selected
  nextUITime = sliceStartTime + 1500 ;
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, MAIN_MENU_BG_COLOUR ) ;
  oled.selectFont( Arial_Black_16 ) ;
  oled.drawString( 20, 50, F("Lock keys"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
}

void handleLockKeys() {
  if ( sliceStartTime >= nextUITime ) {
    // Temp return to main menu
    uiState = UI_DRAW_MAIN_MENU ;
  }
}

void drawMenuLine( const String menuText, const byte lineNum, const boolean selected ) {
  Colour fgColour = MAIN_MENU_FG_COLOUR ;
  Colour bgColour = MAIN_MENU_BG_COLOUR ;
  if ( selected ) {
    fgColour = MAIN_MENU_BG_COLOUR ;
    bgColour = MAIN_MENU_FG_COLOUR ;
  }
  oled.drawString( MF_MIN_X + 1, MF_MAX_Y - ( ( lineNum + 1 ) * MAIN_MENU_LINE_HEIGHT ), menuText, fgColour, bgColour ) ;
}

String getMenuItemText ( const byte itemNum ) {
  char itemText [ MENU_ITEM_TEXT_BUF_LEN ] ;
  String numSelector = "" ;
  numSelector += itemNum + 1 ;
  numSelector += ". " ;
  strcpy_P ( itemText, ( char * ) pgm_read_word ( & ( main_menu_table[ itemNum ] ) ) ) ;
  return numSelector + itemText ;
}

void drawMainMenu() {
  // Menu consists of 7 lines (MAIN_MENU_MAX_LINES) at 16 pixels (MAIN_MENU_LINE_HEIGHT) high
  // If more than 7 items exist in menu, start drawing from offsetItem to effectively scroll
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, MAIN_MENU_BG_COLOUR ) ;
  oled.selectFont( Arial_Black_16 ) ;
  byte offsetItem = 0 ;
  if ( curMenuItem >= MAIN_MENU_MAX_LINES ) {
    offsetItem = curMenuItem - MAIN_MENU_MAX_LINES + 1 ;
  }
  for ( byte lineNum = 0 ; lineNum < MAIN_MENU_MAX_LINES ; lineNum ++ ) {
    if ( lineNum + offsetItem < MAIN_MENU_NUM_ITEMS ) {
      drawMenuLine( getMenuItemText ( lineNum + offsetItem ), lineNum, curMenuItem == lineNum + offsetItem ) ;
    }
  }
  lastMenuItem = curMenuItem ; // track last selected menu item
}

// Draw if something changed
void updateMainMenu() {
  if ( curMenuItem != lastMenuItem ) {
    if (  curMenuItem >= MAIN_MENU_MAX_LINES || lastMenuItem >= MAIN_MENU_MAX_LINES ) {
      // Just redraw whole thing as we're scrolling items
      drawMainMenu() ;
    } else {
      // Just draw the items that have changed
      oled.selectFont( Arial_Black_16 ) ;
      drawMenuLine( getMenuItemText ( lastMenuItem ), lastMenuItem, false ) ;
      drawMenuLine( getMenuItemText ( curMenuItem ), curMenuItem, true ) ;
      lastMenuItem = curMenuItem ;
    }
  }
}

// Power/Start up animation
void startupAnimation() {
  if ( sliceStartTime >= nextUITime ) {
    if ( startupFrameNum == 0 ) {
      // First animation frame
      oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, STARTUP_BG_COLOUR ) ;
      oled.selectFont( Droid_Sans_12 ) ;
    } else if ( startupFrameNum == STARTUP_NUM_FRAMES ) {
      // Last animation frame
      // Unfortunately lots of magic values here to accurately draw oval box around freetronics logo
      oled.drawCircle( 21, MF_MAX_Y - 5 - startupFrameNum, 8, STARTUP_FG_COLOUR ) ;
      oled.drawCircle( 70, MF_MAX_Y - 5 - startupFrameNum, 8, STARTUP_FG_COLOUR ) ;
      oled.drawBox( 23, MF_MAX_Y - STARTUP_TEXT_HEIGHT - 1 - startupFrameNum, 70, MF_MAX_Y + 3 - startupFrameNum, 1, STARTUP_FG_COLOUR ) ;
      oled.drawFilledBox( 23, MF_MAX_Y - 1 - startupFrameNum, 70, MF_MAX_Y + 2 - startupFrameNum, STARTUP_BG_COLOUR ) ;
      oled.drawString( 20, MF_MAX_Y - STARTUP_TEXT_HEIGHT - startupFrameNum, F("freetronics"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      nextUITime = sliceStartTime + STARTUP_HOLD_DELAY ;
    } else if ( startupFrameNum == STARTUP_NUM_FRAMES + 1 ) {
      // Transition to menu
      uiState = UI_DRAW_MAIN_MENU ;
    } else {
      oled.drawLine( 20, MF_MAX_Y - startupFrameNum, 100, MF_MAX_Y - startupFrameNum, STARTUP_BG_COLOUR ) ; // Clear top of previous drawString
      oled.drawString( 20, MF_MAX_Y - STARTUP_TEXT_HEIGHT - startupFrameNum, F("freetronics"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      oled.drawString( 60, MF_MIN_Y + startupFrameNum, F("ArduPhone"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      nextUITime = sliceStartTime + STARTUP_FRAME_PERIOD ;
    }
    startupFrameNum ++ ;
  }
}

// Called from keypad 'module'
void handleKeyPressed( char key ) {

  // Check if screen is powered off and if so, turn back on
  if ( screenState == screen_POWER_OFF ) {
    TurnDisplayOn() ;
  } else {
    switch ( uiState ) {

      case UI_MAIN_MENU :
        if ( key == 'D' ) {
          // Next menu item
          curMenuItem = ( curMenuItem + 1 ) % MAIN_MENU_NUM_ITEMS ;
          updateMainMenu() ;
        } else if ( key == 'U' ) {
          // Previous menu item
          if ( curMenuItem == 0 ) {
            curMenuItem = MAIN_MENU_NUM_ITEMS - 1 ;
          } else {
            curMenuItem -- ;
          }
          updateMainMenu() ;
        } else if ( key == 'R' || key == 'L' ) {
          // Select menu item
          if ( main_menu_functions[ curMenuItem ] ) main_menu_functions[ curMenuItem ]() ;
        } else if ( key >= '1' && key <= '9' ) {
          byte itemSelected = key - '1' ;
          if ( itemSelected < MAIN_MENU_NUM_ITEMS ) {
            curMenuItem = itemSelected ;
            if ( main_menu_functions[ itemSelected ] ) {
              main_menu_functions[ itemSelected ]() ;
            } else {
              updateMainMenu() ;
            }
          }
        }
        break ;

      case UI_GET_PHONE_NUM :
        if ( key == 'U' ) {
          // Number is complete, pass to return state
          phoneNumberEntered = true ;
          uiState = returnNumState ;
        } else if ( key == 'D' || key == 'R' ) {
          // User has cancelled, go back to main menu
          uiState = UI_DRAW_MAIN_MENU ;
        } else if ( key == 'L') {
          // Backspace, loose the last char
          if ( phoneNumBufferIndex > 0 ) {
            phoneNumBuffer[ -- phoneNumBufferIndex ] = '\0' ;
          }
        } else if ( key >= '0' && key <= '9' ) {
          // Add to phone number
          if ( phoneNumBufferIndex < PHONE_NUM_BUF_SIZE - 1 ) {
            phoneNumBuffer[ phoneNumBufferIndex ++ ] = key ;
            phoneNumBuffer[ phoneNumBufferIndex ] = '\0' ;
          }
        } else if ( key == '*' ) {
          // Add a '+' to phone number
          if ( phoneNumBufferIndex < PHONE_NUM_BUF_SIZE - 1 ) {
            phoneNumBuffer[ phoneNumBufferIndex ++ ] = '+' ;
            phoneNumBuffer[ phoneNumBufferIndex ] = '\0' ;
          }
        }
        drawPhoneNumberBuffer() ;
        break ;

      case UI_MAKE_CALL :
        switch ( makeCallState ) {
          case MC_WAITING_FOR_MODEM :
          case MC_DIALLING :
          case MC_ON_CALL :
            if ( key == 'D' || key == 'R' ) {
              // User has cancelled or wants to hang up
              makeCallState = MC_HANG_UP ;
            }
            break ;
          case MC_MODEM_ERROR :
            // Abort showing error and go to menu
            uiState = UI_DRAW_MAIN_MENU ;
            break ;
        }
        if ( makeCallState == MC_MODEM_ERROR ) {
          // Abort showing error and go straight to menu
          uiState = UI_DRAW_MAIN_MENU ;
        }
        break ;

      case UI_RCV_CALL :
        switch ( receiveCallState ) {
          case RC_WAITING_FOR_ACCEPT :
            if ( key == 'U' || key == 'L' ) {
              // Answer the call
              receiveCallState = RC_WAITING_FOR_MODEM ; // which will then answer call
            } else if ( key == 'R' || key == 'D' ) {
              // Hang up / Ignore the call
              uiState = UI_DRAW_MAIN_MENU ;
            }
            break ;

          case RC_ON_CALL :
            if ( key == 'R' || key == 'D' ) {
              // Hang up
              SendGSMSerial( F("ATH\r") ) ;
              displayHungUp() ;
            }
            break ;
        }
        break ;

        case UI_SEND_SMS:
        {
            if ( key == 'U' ) {
                /// up == done
                smsEntered = true;
                uiState = returnNumState;
            } else if (key == 'L') {
                /// left == delete
                if (smsBufferIndex)
                {
                    --smsBufferIndex;
                }
            } else if (key == 'R') {
                /// right == CANCEL
                smsBufferIndex = 0;
                uiState = UI_DRAW_MAIN_MENU;
            } else if (key == 'D') {
                /// right == CLEAR
                smsBufferIndex = 0;
            } else {
                bool del;
                key = translate(key,del);
                if ((smsBufferIndex >= SMS_BUF_SIZE)
                    ||
                    (del && phoneNumBufferIndex))
                {
                    --smsBufferIndex;
                }
                smsBuffer[smsBufferIndex++] = key;
            }
            drawSMSBuffer();
        } break;
      case UI_START_UP :
        // Abort animation and go to menu
        uiState = UI_DRAW_MAIN_MENU ;
        break ;

      // Else ignore the key
    }
  nextScreenOffTime = sliceStartTime + SCREEN_POWER_OFF_DELAY ; // Delay screen powering off
  }
}

// =======================

void UISetup() {
  uiState = UI_UNINITIALISED ;
  nextUITime = sliceStartTime + 200 ; // wait breifly before starting UI


}

// =======================

void UISlice() {
  switch ( uiState ) {

    case UI_UNINITIALISED :
      uiState = UI_START_UP ;
      break ;

    case UI_START_UP :
      startupAnimation() ;
      break ;

    case UI_DRAW_MAIN_MENU :
      drawMainMenu() ;
      uiState = UI_MAIN_MENU ;
      break ;

    case UI_MAIN_MENU :
      // Don't do anything as handled by key press event
      break ;

    case UI_MAKE_CALL :
      handleMakeCall() ;
      break ;

    case UI_RCV_CALL :
      handleReceiveCall() ;
      break ;

    case UI_GET_PHONE_NUM :
      // Don't do anything as handled by key press event
      break ;

    case UI_SEND_SMS :
      handleSendSMS() ;
      break ;

    case UI_LOCK_KEYS :
        handleLockKeys();
      break ;
  }
}
