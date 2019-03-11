// Example showing use of the MD_UISwitch library
// 
// Exercises different types of UI switches 
// 
// Prints the switch value on the Serial Monitor
// Allows setting of options to see their effect (see setup())
//
#include <MD_UISwitch.h>

// Define what type of testing is being done
#define TEST_DIGITAL_SIMPLE 1
#define TEST_DIGITAL_ARRAY 0

#define TEST_ANALOG  0

#define TEST_MATRIX_4b4  0
#define TEST_MATRIX_1b6  0

#define TEST_MATRIX_4017KM 0


#if TEST_DIGITAL_SIMPLE
#define TITLE "Simple Digital"

const uint8_t DIGITAL_SWITCH_PIN = 4;       // switch connected to this pin
const uint8_t DIGITAL_SWITCH_ACTIVE = LOW;  // digital signal when switch is pressed 'on'

MD_UISwitch_Digital S(DIGITAL_SWITCH_PIN, DIGITAL_SWITCH_ACTIVE);
#endif

#if TEST_DIGITAL_ARRAY
#define TITLE "Array Digital"
const uint8_t DIGITAL_SWITCH_PINS[] = { 4, 5, 6 }; // switches connected to these pins
const uint8_t DIGITAL_SWITCH_ACTIVE = LOW;  // digital signal when switch is pressed 'on'

MD_UISwitch_Digital S(DIGITAL_SWITCH_PINS, ARRAY_SIZE(DIGITAL_SWITCH_PINS), DIGITAL_SWITCH_ACTIVE);
#endif

#if TEST_ANALOG
#define TITLE "Analog Input"

const uint8_t ANALOG_SWITCH_PIN = A0;       // switches connected to this pin

// These key values work for most LCD shields
MD_UISwitch_Analog::uiAnalogKeys_t kt[] =
{
  {  10, 10, 'R' },  // Right
  { 130, 15, 'U' },  // Up
  { 305, 15, 'D' },  // Down
  { 475, 15, 'L' },  // Left
  { 720, 15, 'S' },  // Select
};

MD_UISwitch_Analog S(ANALOG_SWITCH_PIN, kt, ARRAY_SIZE(kt));
#endif

#if TEST_MATRIX_4b4
#define TITLE "Matrix 4x4"

uint8_t rowPins[] = { 4, 5, 6, 7 };     // connected to keypad row pinouts
uint8_t colPins[] = { 8, 9, 10, 11 };   // connected to the keypad column pinouts

const uint8_t ROWS = sizeof(rowPins);
const uint8_t COLS = sizeof(colPins);

char kt[(ROWS*COLS) + 1] = "123A456B789C*0#D";  //define the symbols for the keypad

MD_UISwitch_Matrix S(ROWS, COLS, rowPins, colPins, kt);
#endif

#if TEST_MATRIX_1b6
#define TITLE "Matrix 1x6"

uint8_t rowPins[] = { 11 };     // connected to keypad row pinouts
uint8_t colPins[] = { 10, 9, 8, 7, 6, 5 };   // connected to the keypad column pinouts

const uint8_t ROWS = sizeof(rowPins);
const uint8_t COLS = sizeof(colPins);

char kt[(ROWS*COLS) + 1] = "AMLRSO";  //define the symbols for the keypad

MD_UISwitch_Matrix S(ROWS, COLS, rowPins, colPins, kt);
#endif

#if TEST_MATRIX_4017KM
#define TITLE "Matrix 4017KM"

const uint8_t KM4017_CLK = 3;  // clock connected to this pin
const uint8_t KM4017_RST = 4;  // reset connectred to this pin
const uint8_t KM4017_OUT = 5;  // output from keypad connected here

MD_UISwitch_4017KM S(9, KM4017_CLK, KM4017_OUT, KM4017_RST);
#endif

void setup(void)
{
  Serial.begin(57600);
  Serial.print(F("\n[MD_UISwitch "));
  Serial.print(F(TITLE));
  Serial.print(F(" Example]"));

  S.begin();
  //S.enableDoublePress(false);
  //S.enableLongPress(false);
  //S.enableRepeat(false);
  S.enableRepeatResult(true);
}

void loop(void)
{
  MD_UISwitch::keyResult_t k = S.read();

  switch(k)
  {
    case MD_UISwitch::KEY_NULL:      /* Serial.print("KEY_NULL"); */  break;
    case MD_UISwitch::KEY_UP:        Serial.print("\nKEY_UP ");     break;
    case MD_UISwitch::KEY_DOWN:      Serial.print("\n\nKEY_DOWN ");   break;
    case MD_UISwitch::KEY_PRESS:     Serial.print("\nKEY_PRESS ");  break;
    case MD_UISwitch::KEY_DPRESS:    Serial.print("\nKEY_DOUBLE "); break;
    case MD_UISwitch::KEY_LONGPRESS: Serial.print("\nKEY_LONG   "); break;
    case MD_UISwitch::KEY_RPTPRESS:  Serial.print("\nKEY_REPEAT "); break;
    default:                         Serial.print("\nKEY_UNKNWN "); break;
  }
  if (k != MD_UISwitch::KEY_NULL)
  {
    if (S.getKey() >= ' ')
    {
      Serial.print((char)S.getKey());
      Serial.print(" ");
    }
    Serial.print("[0x");
    Serial.print(S.getKey(), HEX);
    Serial.print("]");
  }
}
