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
const unsigned int STARTUP_HOLD_DELAY = 2000 ; // How long to wait after animation completes
const byte STARTUP_NUM_FRAMES = 38 ;
const byte STARTUP_TEXT_HEIGHT = 12 ;
const Colour STARTUP_BG_COLOUR = { 0, 0, 1 } ;
const Colour STARTUP_FG_COLOUR = YELLOW ;

// === States ===

enum uiStates {
  UI_UNINITIALISED,
  UI_START_UP,
  UI_MAIN_MENU
} ;
uiStates uiState ;

// === Variables ===

unsigned long nextUITime ;
byte startupFrameNum = 0 ;


// === Functions ===

// Start up animation
void startupAnimation() {
  if ( sliceStartTime >= nextUITime ) {
    if ( startupFrameNum == 0 ) {
      // First animation frame
      oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, STARTUP_BG_COLOUR ) ;
      oled.selectFont( Droid_Sans_12 ) ;
    } else if ( startupFrameNum == STARTUP_NUM_FRAMES ) {
      // Last animation frame
      oled.drawCircle( 21, MF_MAX_Y - 5 - startupFrameNum, 8, STARTUP_FG_COLOUR ) ;
      oled.drawCircle( 70, MF_MAX_Y - 5 - startupFrameNum, 8, STARTUP_FG_COLOUR ) ;
      oled.drawBox( 23, MF_MAX_Y - STARTUP_TEXT_HEIGHT - 1 - startupFrameNum, 70, MF_MAX_Y + 3 - startupFrameNum, 1, STARTUP_FG_COLOUR ) ;
      oled.drawFilledBox( 23, MF_MAX_Y - 1 - startupFrameNum, 70, MF_MAX_Y + 2 - startupFrameNum, STARTUP_BG_COLOUR ) ;
      oled.drawString( 20, MF_MAX_Y - STARTUP_TEXT_HEIGHT - startupFrameNum, F("freetronics"), STARTUP_FG_COLOUR, STARTUP_BG_COLOUR ) ;
      nextUITime = sliceStartTime + STARTUP_HOLD_DELAY ;
    } else if ( startupFrameNum == STARTUP_NUM_FRAMES + 1 ) {
      // Transition to menu
      uiState = UI_MAIN_MENU ;
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
      
    case UI_MAIN_MENU :
      break ;
  }
}
