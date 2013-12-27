/*
 * Freetronics ArduPhone - screen
 *
 * Functions related to the OLED128 screen
 *
 * Cary Dreelan - Dec 2013
 */

// === Constants ===

const byte OLED_PIN_CS = 4 ;
const byte OLED_PIN_DC = 7 ;
const byte OLED_PIN_RESET = 5 ;
const byte SCREEN_UPDATE_PERIOD = 50 ; // in ms ( 1000 / 50 = 20 fps)
// Main frame co-ords
const byte MF_MIN_X = 0 ;
const byte MF_MIN_Y = 0 ;
const byte MF_MAX_X = 127 ;
const byte MF_MAX_Y = 112 ;
// Status bar co-ords
const byte SB_MIN_X = 0 ;
const byte SB_MIN_Y = 113 ; // Effectively 14 pixels high
const byte SB_MAX_X = 127 ;
const byte SB_MAX_Y = 126 ; // - 1 to stop wrap around to bottom bug?
const Colour SB_BG_COLOUR = WHITE ;
const byte SINE_FRAMES = 12 ; // Must match array size used by sineIndex (below)
// Phone activity animation
const byte ACTIVITY_MIN_X = SB_MAX_X - 12 ; 
const byte ACTIVITY_MAX_X = SB_MAX_X - 4 ; // allows for + box size
const byte ACTIVITY_BOX_SIZE = 3 ;
// GSM Display status (effectively left 80 pixels of status bar)
const byte GSM_MIN_X = SB_MIN_X ;
const byte GSM_MIN_Y = SB_MIN_Y + 1 ;
const byte GSM_MAX_X = SB_MIN_X + 80 ;
const byte GSM_MAX_Y = SB_MAX_Y ;

// === States ===

enum screenStates {
  screen_UNINITIALISED,
  screen_SETUP,
  screen_UPDATE,
  screen_POWER_OFF,
  screen_POWER_ON
} ;
screenStates screenState ;

// === Variables ===

OLED oled( OLED_PIN_CS, OLED_PIN_DC, OLED_PIN_RESET ) ;
unsigned long lastScreenTime ;
byte statusActivityFrame = 0 ;
byte sineIndex [ SINE_FRAMES ] = { 0, 1, 2, 4, 6, 7, 8, 7, 6, 4, 2, 1 } ;
byte activityIndexX = 0 ;
byte activityIndexY = SINE_FRAMES / 2 ;
boolean activityOrder = false ;

// === Functions ===

void DrawActivityBoxes( byte xAmt, byte yAmt, Colour box1Colour, Colour box2Colour ) {
  oled.drawFilledBox( ACTIVITY_MIN_X + xAmt, 
                      SB_MIN_Y + 1 + yAmt, 
                      ACTIVITY_MIN_X + xAmt + ACTIVITY_BOX_SIZE, 
                      SB_MIN_Y + 1  + yAmt + ACTIVITY_BOX_SIZE, 
                      box1Colour ) ;
  oled.drawFilledBox( ACTIVITY_MAX_X - xAmt, 
                      SB_MAX_Y - 1 - yAmt, 
                      ACTIVITY_MAX_X - xAmt + ACTIVITY_BOX_SIZE, 
                      SB_MAX_Y - 1 - yAmt - ACTIVITY_BOX_SIZE, 
                      box2Colour ) ;
}

void UpdateActivityAnimation() {
  // Erase old first by drawing background colour
  byte xAmt = sineIndex [ activityIndexX ] ;
  byte yAmt = sineIndex [ activityIndexY ] ;
  DrawActivityBoxes( xAmt, yAmt, SB_BG_COLOUR, SB_BG_COLOUR ) ;
  // Calulate and draw position for next frame
  activityIndexX = ( activityIndexX + 1 ) % SINE_FRAMES ;
  if ( activityIndexX % 3 == 0 ) {
    activityIndexY = ( activityIndexY + 1 ) % SINE_FRAMES ;
  }
  xAmt = sineIndex [ activityIndexX ] ;
  yAmt = sineIndex [ activityIndexY ] ;
  DrawActivityBoxes( xAmt, yAmt, RED, BLUE ) ;
}

void DrawGSMSignalStrength() {
  // Remove this number and use graph below only?
  String msg = "" ;
  msg += gsmSignalStrength ;
  oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, msg, BLUE, SB_BG_COLOUR ) ;
  
  // gsmSignalStrength varies from 0-31 as returned by the gsm module
  Colour meterColour = GREEN ;
  int lx, ly = 0 ;
  for ( byte count = 0 ; count <= 31 ; count ++ ) {
    lx = GSM_MIN_X + 1 + count ;
    ly = GSM_MIN_Y + 1 + count / 3 ;
    oled.drawLine( lx, GSM_MIN_Y + 1, lx, ly, meterColour ) ;
    if ( count == gsmSignalStrength ) meterColour = BLACK ;
  }
  // Set last displayed signal strength
  gsmDisplayedSignalStrength = gsmSignalStrength ;
}

// Draw status of GSM module, inc aerial strength, if changed
void UpdateGSMStatusDisplay() {
  if ( gsmDisplayStatus != gsmLastDisplayedStatus || gsmSignalStrength != gsmDisplayedSignalStrength ) {
    oled.drawFilledBox( GSM_MIN_X, GSM_MIN_Y, GSM_MAX_X, GSM_MAX_Y, SB_BG_COLOUR ) ;
    oled.selectFont( Droid_Sans_12 ) ;

    switch ( gsmDisplayStatus ) {

      case gsmd_HARDWARE_OFF :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("GSM OFF"), RED, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_POWERING_ON_1 :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("POWER ON 1"), ORANGE, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_POWERING_ON_2 :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("POWER ON 2"), ORANGE, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_POWERING_ON_3 :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("POWER ON 3"), ORANGE, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_POWERING_ON_4 :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("POWER ON 4"), ORANGE, SB_BG_COLOUR ) ;
        break ;
        
      case gsmd_UNKNOWN :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("Enquiring"), BLUE, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_NORMAL :
        // Draw aerial strength
        DrawGSMSignalStrength() ;
        // Draw Carrier name if possible?
        // - TODO
        break ;
  
      case gsmd_NO_SIM :
        // Draw aerial strength
        DrawGSMSignalStrength() ;
        oled.drawString( GSM_MIN_X + 35, GSM_MIN_Y, F("NO SIM"), RED, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_POWER_OFF_SOFT :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("A/plane mode"), RED, SB_BG_COLOUR ) ;
        break ;
  
      case gsmd_POWERING_ON_SOFT :
        oled.drawString( GSM_MIN_X + 1, GSM_MIN_Y, F("Turning on"), RED, SB_BG_COLOUR ) ;
        break ;
    }
    gsmLastDisplayedStatus = gsmDisplayStatus ;
  }
}

// Setup and draw everyting related to status bar
void DrawStatusBar() {
  oled.drawFilledBox( SB_MIN_X, SB_MIN_Y, SB_MAX_X, SB_MAX_Y, SB_BG_COLOUR ) ;
  UpdateActivityAnimation() ;
  UpdateGSMStatusDisplay() ;
}

// Draw if anything has changed since last update
void UpdateStatusBar() {
  UpdateActivityAnimation() ;
  UpdateGSMStatusDisplay() ;
}

// Setup and draw everyting related to main frame
void DrawMainFrame() {
  oled.drawFilledBox( MF_MIN_X, MF_MIN_Y, MF_MAX_X, MF_MAX_Y, BLACK ) ;
}

// Draw if anything has changed since last update
void UpdateMainFrame() {
}

void TurnDisplayOff() {
  oled.setDisplayOn( false ) ;
  screenState = screen_POWER_OFF ;
}

void TurnDisplayOn() {
  oled.setDisplayOn( true ) ;
  screenState = screen_POWER_ON ;
}

// =======================

void ScreenSetup() {
  screenState = screen_UNINITIALISED ;
}

// =======================

void ScreenSlice() {
  switch ( screenState ) {
    
    case screen_UPDATE :
      if ( ( sliceStartTime - lastScreenTime ) > SCREEN_UPDATE_PERIOD ) {
        UpdateStatusBar() ;
        UpdateMainFrame() ;
        lastScreenTime = sliceStartTime ;
      }
      break ;

    case screen_UNINITIALISED :
      oled.begin() ;
      oled.setOrientation(ROTATE_180) ;
      screenState = screen_SETUP ;
      break ;
      
    case screen_SETUP :
      DrawStatusBar() ;
      DrawMainFrame() ;
      screenState = screen_UPDATE ;
      break ;
      
    case screen_POWER_OFF :
      // Do nothing
      break ;

    case screen_POWER_ON :
      // Resume drawing
      screenState = screen_UPDATE ;
      break ;

  }
}
