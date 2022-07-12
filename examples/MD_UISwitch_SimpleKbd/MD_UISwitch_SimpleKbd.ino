// Example showing use of the MD_UISwitch library
// 
// Implements simple numeric entry from keypad
// - Number is a string of digits in the specified base followed by the # character
// - Clear current input by pressing *
// - Displays the number entered on the Serial Monitor
//
#include <MD_UISwitch.h>

// Define what type of keypad is being used
#define MATRIX_3b4  0
#define MATRIX_4b4  1

// Define the base of the input number
#define INPUT_DECIMAL 0   
#define INPUT_HEX     1

#if MATRIX_3b4
#define TITLE "3x4"

uint8_t colPins[] = { 2, 3, 4 };      // connected to keypad column 1 to 3 pinouts
uint8_t rowPins[] = { 5, 6, 7, 8 };   // connected to the keypad row 1 to 4 pinouts

const uint8_t ROWS = sizeof(rowPins) * sizeof(uint8_t);
const uint8_t COLS = sizeof(colPins) * sizeof(uint8_t);

char kt[(ROWS * COLS) + 1] = "123456789*0#";  //define the symbols for the keypad by row
#endif

#if MATRIX_4b4
#define TITLE "4x4"

uint8_t colPins[] = { 2, 3, 4, 5 };   // connected to keypad column 1 to 4 pinouts
uint8_t rowPins[] = { 6, 7, 8, 9 };   // connected to the keypad row 1 to 4 pinouts

const uint8_t ROWS = sizeof(rowPins) * sizeof(uint8_t);
const uint8_t COLS = sizeof(colPins) * sizeof(uint8_t);

char kt[(ROWS * COLS) + 1] = "123A456B789C*0#D";  //define the symbols for the keypad by row
#endif

MD_UISwitch_Matrix S(ROWS, COLS, rowPins, colPins, kt);

void setup(void)
{
  Serial.begin(57600);
  Serial.print(F("\n[MD_UISwitch Matrix "));
  Serial.print(F(TITLE));
  Serial.print(F(" Example]"));

  S.begin();
  S.enableDoublePress(false);
  S.enableLongPress(false);
  S.enableRepeat(false);
  S.enableRepeatResult(false);
}

uint32_t inputNumber = 0;
uint32_t inputTemp = 0;

bool readKbd(void)
{
  bool r = false;
  char c;
  MD_UISwitch::keyResult_t k = S.read();

  if (k == MD_UISwitch::KEY_PRESS)
  {
    c = toupper((char)S.getKey());

    switch (c)
    {
    case '*':        // clear input 
      inputTemp = 0; 
      break;

    case '#':        // end input 
      inputNumber = inputTemp; 
      inputTemp = 0;
      r = true;      
      break;

    default:
      {
#if INPUT_DECIMAL
      if (c >= '0' && c <= '9')
      {
        inputTemp *= 10; 
        inputTemp += (c - '0');
      }
#endif
#if INPUT_HEX
        uint8_t x = 99; // assigned invalid value

        if (c >= '0' && c <= '9') x = c - '0';
        else if (c >= 'A' && c <= 'F') x = 10 + (c - 'A');
        if (x != 99)
        {
          inputTemp <<= 4;
          inputTemp += x;
        }
#endif
      break;
      }
    }
  }
  return(r);
}

void loop(void)
{
  if (readKbd())
  {
    Serial.print(F("\n"));
    Serial.print(inputNumber);
    Serial.print(F(" 0x"));
    Serial.print(inputNumber, HEX);
  }
}
