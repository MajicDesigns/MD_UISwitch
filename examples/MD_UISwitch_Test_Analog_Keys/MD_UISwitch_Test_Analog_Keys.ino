// Utility program for MD_UISwitch library
//
// Determine the calibration values for the keys on 
// the LCD display menu pad, display the values on 
// the LCD or in the serial stream
//
#include <LiquidCrystal.h>

#define DISPLAY_LCD 1

const uint8_t ANALOG_KEYS = A0;

#if DISPLAY_LCD
// LCD display definitions
const uint8_t LCD_ROWS = 2;
const uint8_t LCD_COLS = 16;

// LCD pin definitions
// These may change depending on your LCD display
const uint8_t LCD_RS = 8;
const uint8_t LCD_ENA = 9;
const uint8_t LCD_D4 = 4;
const uint8_t LCD_D5 = LCD_D4+1;
const uint8_t LCD_D6 = LCD_D4+2;
const uint8_t LCD_D7 = LCD_D4+3;

LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
#define DEVICE lcd
#else
#define DEVICE Serial
#endif

void setup() 
{ 
#if DISPLAY_LCD
  DEVICE.begin(LCD_COLS, LCD_ROWS);
  DEVICE.clear();
#else
  DEVICE.begin(57600);
#endif

  DEVICE.print("Keys Calibrate");
#if DISPLAY_LCD
  DEVICE.setCursor(0,1);
#endif
  DEVICE.print("Press keys ...");

  pinMode(ANALOG_KEYS, INPUT);
}

void loop() 
{
  uint16_t adcKeyIn;

  adcKeyIn = analogRead(ANALOG_KEYS);    // read the value from the port  
  
#if DISPLAY_LCD
  DEVICE.home();
#endif
  DEVICE.print("Value = ");
  DEVICE.print(adcKeyIn);
#if DISPLAY_LCD
  DEVICE.print("   ");
#else
  DEVICE.println();
#endif

  delay(100);
}

