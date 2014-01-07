// -*- Mode:c++ -*-
/*
 * Freetronics ArduPhone - ui
 *
 * Functions related to running the user interface.
 *
 * Cary Dreelan - Dec 2013
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
const unsigned int MAKE_CALL_ERROR_DISPLAY_TIME = 3000 ; // How long to display error when dialilng

// === States ===

// uiStates enum moved to ui.h due to Arduino IDE limitation when enum used as function parameter
uiStates uiState, returnNumState ;
enum makeCallStates {
  MC_DRAW_CALLING_NUMBER,
  MC_WAITING_FOR_MODEM,
  MC_DIALLING,
  MC_MODEM_ERROR,
  MC_ON_CALL,
  MC_HANG_UP,
} ;
makeCallStates makeCallState ;

// === Variables ===

unsigned long nextUITime ;
byte startupFrameNum = 0 ;
// Main menu variables - these next 3 items and their arrays must be in sync
prog_char mainMenu0[] PROGMEM = "Call";
prog_char mainMenu1[] PROGMEM = "SMS";
prog_char mainMenu2[] PROGMEM = "Lock keys";
prog_char mainMenu3[] PROGMEM = "Four";
prog_char mainMenu4[] PROGMEM = "Five";
prog_char mainMenu5[] PROGMEM = "Six";
prog_char mainMenu6[] PROGMEM = "Seven";
prog_char mainMenu7[] PROGMEM = "Eight";
prog_char mainMenu8[] PROGMEM = "Nine";
prog_char mainMenu9[] PROGMEM = "Ten";
const byte MAIN_MENU_NUM_ITEMS = 10 ;
PROGMEM const char * main_menu_table[] = {
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
typedef void ( * FunctionPointer ) ();
FunctionPointer main_menu_functions[] = {
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
byte phoneNumBufferIndex ;
boolean phoneNumberEntered = false ;

// === Functions ===

// Call related functions

void drawPhoneNumberBuffer() {
  // Draw centered phone number with space before and after (needed incase of backspace)
  oled.selectFont( Arial_Black_16 ) ;
  char displayNumber[ PHONE_NUM_BUF_SIZE + 3 ] ; // To allow for a ' ' before and after
  displayNumber[ 0 ] = ' ' ;
  strncpy( displayNumber + 1, phoneNumBuffer, PHONE_NUM_BUF_SIZE ) ;
  displayNumber[ phoneNumBufferIndex + 1 ] = ' ' ;
  displayNumber[ phoneNumBufferIndex + 2 ] = '\0' ;
  byte posX = ( MF_MAX_X - oledStringWidth( displayNumber ) ) / 2 ;
  oled.drawString( posX, PHONE_NUM_BUF_MIN_Y, displayNumber, MAKE_CALL_NUM_COLOUR, MAKE_CALL_BG_COLOUR ) ;
}

void setupGetPhoneNumer( const uiStates returnState ) {
  phoneNumberEntered = false ;
  phoneNumBuffer[ 0 ] = '\0' ;
  phoneNumBufferIndex = 0 ;
  returnNumState = returnState ;
  uiState = UI_GET_PHONE_NUM ;
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
  setupGetPhoneNumer( UI_MAKE_CALL ) ;
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
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, MAIN_MENU_BG_COLOUR ) ;
  oled.selectFont( Arial_Black_16 ) ;
  oled.drawString( 15, 50, F("Create SMS"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
}

void handleCreateSMS() {
  if ( sliceStartTime >= nextUITime ) {
    // Temp return to main menu
    uiState = UI_DRAW_MAIN_MENU ;
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

    case UI_GET_PHONE_NUM :
      // Don't do anything as handled by key press event
      break ;

    case UI_CREATE_SMS :
      handleCreateSMS() ;
      break ;

    case UI_LOCK_KEYS :
      handleCreateSMS() ;
      break ;
  }
}
