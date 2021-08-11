// Example showing use of the MD_UISwitch library
// 
// Exercises User defined digital input 
// 
// Prints the switch value on the Serial Monitor
// Allows setting of options to see their effect (see setup())
//
#include <MD_UISwitch.h>

const uint8_t DIGITAL_SWITCH_PIN = 4;       // switch connected to this pin
const uint8_t DIGITAL_SWITCH_ACTIVE = LOW;  // digital signal when switch is pressed 'on'

bool SwCallback(uint8_t id)
// Callback from the library to obtain the user value for the 
// identified switch. Returns true if the switch is active, 
// false if not active.
{
  bool b = false;

  // insert the logic to get the value identified by the ID
  if (id == DIGITAL_SWITCH_PIN)
    b = (digitalRead(id) == DIGITAL_SWITCH_ACTIVE);
  else
  {
    Serial.print("\nRequest for undefined id ");
    Serial.print(id);
  }

  return(b);
}

MD_UISwitch_User S(DIGITAL_SWITCH_PIN, SwCallback);

void setup(void)
{
  Serial.begin(57600);
  Serial.print(F("\n[MD_UISwitch User Example]"));

  pinMode(DIGITAL_SWITCH_PIN, (DIGITAL_SWITCH_ACTIVE == LOW) ? INPUT_PULLUP : INPUT);

  S.begin();
  //S.enableDoublePress(false);
  //S.enableLongPress(false);
  //S.enableRepeat(false);
  S.enableRepeatResult(true);
}

void loop(void)
{
  
  MD_UISwitch::keyResult_t k;
  
  k = S.read();

  switch(k)
  {
    case MD_UISwitch::KEY_NULL:      /* Serial.print("KEY_NULL"); */  break;
    case MD_UISwitch::KEY_UP:        Serial.print("\nKEY_UP ");     break;
    case MD_UISwitch::KEY_DOWN:      Serial.print("\nKEY_DOWN ");   break;
    case MD_UISwitch::KEY_PRESS:     Serial.print("\nKEY_PRESS ");  break;
    case MD_UISwitch::KEY_DPRESS:    Serial.print("\nKEY_DOUBLE "); break;
    case MD_UISwitch::KEY_LONGPRESS: Serial.print("\nKEY_LONG "); break;
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
