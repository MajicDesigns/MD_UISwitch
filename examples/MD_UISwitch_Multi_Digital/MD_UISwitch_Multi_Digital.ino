// Example showing use of the MD_UISwitch library
// 
// Exercises many individual digital switches, prints the 
// switch value on the Serial Monitor
//
#include <MD_UISwitch.h>

// define pin numbers for individual switches
const uint8_t SW_PIN[] = { 4, 5, 6, 7 };

MD_UISwitch_Digital *BTN[ARRAY_SIZE(SW_PIN)];

void logButtonState(const char* buttonName, MD_UISwitch::keyResult_t state)
{
  if (state == MD_UISwitch::KEY_NULL)
    return;

  Serial.print("\n");
  Serial.print(buttonName);
  Serial.print(": ");

  switch (state)
  {
  case MD_UISwitch::KEY_NULL:      /*Serial.print("KEY_NULL");*/  break;
  case MD_UISwitch::KEY_UP:        Serial.print("KEY_UP");        break;
  case MD_UISwitch::KEY_DOWN:      Serial.print("KEY_DOWN");      break;
  case MD_UISwitch::KEY_PRESS:     Serial.print("KEY_PRESS");     break;
  case MD_UISwitch::KEY_DPRESS:    Serial.print("KEY_DPRESS");    break;
  case MD_UISwitch::KEY_LONGPRESS: Serial.print("KEY_LONGPRESS"); break;
  case MD_UISwitch::KEY_RPTPRESS:  Serial.print("KEY_RPTPRESS");  break;
  default:                         Serial.print("KEY_UNKNWN");    break;
  }
}

void setup(void)
{
  Serial.begin(57600);

  for (uint8_t i = 0; i < ARRAY_SIZE(BTN); i++)
  {
    BTN[i] = new MD_UISwitch_Digital(SW_PIN[i], LOW);
    BTN[i]->begin();
  }
}

void loop(void)
{
  for (uint8_t i = 0; i < ARRAY_SIZE(BTN); i++)
  {
    char name[] = "B ";

    name[1] = i + '0';  // turn into a digit
    logButtonState(name, BTN[i]->read());
  }
}
