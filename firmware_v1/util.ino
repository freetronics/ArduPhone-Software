/*
 * Freetronics ArduPhone - util
 *
 * Generic functions that don't fit elsewhere
 *
 * Cary Dreelan - Dec 2013
 */

// oledStringWidth
// - Currently FTOLED library only has charWidth for current font
// - Iterate through each char and add up its width
byte oledStringWidth( const char * strBuffer ) {
  byte result = 0, index = 0 ;
  char curChar ;
  while ( ( curChar = strBuffer [ index ++ ] ) != 0 ) {
    result += oled.charWidth( curChar ) ; + 1 ; // FTOLED drawstring adds 1 pixel between chars
  }
  return result ;
}
