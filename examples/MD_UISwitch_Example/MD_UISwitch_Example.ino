// Example showing use of the MD_UISwitch library
// 
// Exercises different types of UI switches 
// 
// Prints the switch value on the Serial Monitor
// Allows setting of options to see their effect (see setup())
//
#include <MD_UISwitch.h>

// define what trype of testing is beig done
#define TEST_DIGITAL 1
#define TEST_ANALOG  0
#define TEST_MATRIX_4b4  0
#define TEST_MATRIX_1b4  0

#if TEST_DIGITAL
const uint8_t DIGITAL_SWITCH_PIN = 4;       // switch connected to this pin
const uint8_t DIGITAL_SWITCH_ACTIVE = LOW;  // digital signal when switch is pressed 'on'

MD_UISwitch_Digital S(DIGITAL_SWITCH_PIN, DIGITAL_SWITCH_ACTIVE);
#endif

#if TEST_ANALOG
const uint8_t ANALOG_SWITCH_PIN = A0;       // switches connected to this pin

// These key values work for most LCD shields
MD_UISwitch_Analog::uiAnalogKeys_t kt[] =
{
  {  10, 10, 'R' },  // Right
  { 139, 15, 'U' },  // Up
  { 315, 15, 'D' },  // Down
  { 489, 15, 'L' },  // Left
  { 726, 15, 'S' },  // Select
};

MD_UISwitch_Analog S(ANALOG_SWITCH_PIN, kt, ARRAY_SIZE(kt));
#endif

#if TEST_MATRIX_4b4
uint8_t rowPins[] = { 4, 5, 6, 7 };     // connected to keypad row pinouts
uint8_t colPins[] = { 8, 9, 10, 11 };   // connected to the keypad column pinouts

const uint8_t ROWS = sizeof(rowPins);
const uint8_t COLS = sizeof(colPins);

char kt[(ROWS*COLS) + 1] = "123A456B789C*0#D";  //define the symbols for the keypad

MD_UISwitch_Matrix S(ROWS, COLS, rowPins, colPins, kt);
#endif

#if TEST_MATRIX_1b4
uint8_t rowPins[] = { 11 };     // connected to keypad row pinouts
uint8_t colPins[] = { 10, 9, 8, 7, 6, 5 };   // connected to the keypad column pinouts

const uint8_t ROWS = sizeof(rowPins);
const uint8_t COLS = sizeof(colPins);

char kt[(ROWS*COLS) + 1] = "AMLRSO";  //define the symbols for the keypad

MD_UISwitch_Matrix S(ROWS, COLS, rowPins, colPins, kt);
#endif

void setup(void)
{
  Serial.begin(57600);
  Serial.print("\n[MD_UISwitch example]");

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
    case MD_UISwitch::KEY_NULL:      /* Serial.println("NULL"); */   break;
    case MD_UISwitch::KEY_PRESS:     Serial.print("\nSINGLE PRESS "); break;
    case MD_UISwitch::KEY_DPRESS:    Serial.print("\nDOUBLE PRESS "); break;
    case MD_UISwitch::KEY_LONGPRESS: Serial.print("\nLONG PRESS ");   break;
    case MD_UISwitch::KEY_RPTPRESS:  Serial.print("\nREPEAT PRESS "); break;
    default:                         Serial.print("\nUNKNOWN ");      break;
  }
  if (k != MD_UISwitch::KEY_NULL)
  {
    Serial.print((char)S.getKey());
    Serial.print(" [");
    Serial.print(S.getKey());
    Serial.print("]");
  }
}
