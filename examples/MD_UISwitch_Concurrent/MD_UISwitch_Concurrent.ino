// Example showing use of the MD_UISwitch library
// 
// Detects when multiple switches are at TARGET_STATE at the same time.
// 'At the same time' means within the time set by DETECT_TIME.
//
#include <MD_UISwitch.h>

const uint32_t DETECT_TIME = 200;   // time in ms
const MD_UISwitch::keyResult_t TARGET_STATE = MD_UISwitch::KEY_PRESS;

// Define a structure to keep track of individual pins
typedef struct
{
  uint8_t pin;                    // pin number
  uint32_t timeTriggered;         // last time this was triggered
  MD_UISwitch::keyResult_t state; // last non-null key status
  MD_UISwitch_Digital *sw;        // switch state detection
} switchTracker_t;

// INItialise static data for individual switches
switchTracker_t SW[] =
{
  { 4, 0, MD_UISwitch::KEY_NULL, nullptr },
  { 5, 0, MD_UISwitch::KEY_NULL, nullptr },
  { 6, 0, MD_UISwitch::KEY_NULL, nullptr },
  { 7, 0, MD_UISwitch::KEY_NULL, nullptr },
};

void logSwitchState(void)
{
  for (uint8_t i = 0; i < ARRAY_SIZE(SW); i++)
  {
    MD_UISwitch::keyResult_t k = SW[i].sw->read();

    // save the current state if not NULL
    if (k != MD_UISwitch::KEY_NULL)
    {
      SW[i].state = k;
      SW[i].timeTriggered = millis();
    }
  }
}

void checkConcurrentState(void)
{
  uint8_t count = 0;

  for (uint8_t i = 0; i < ARRAY_SIZE(SW); i++)
  {
    // check if the detection time has expired for this switch
    // and reset the current state if it has
    if (millis() - SW[i].timeTriggered >= DETECT_TIME)
      SW[i].state = MD_UISwitch::KEY_NULL;
    else
      // if this one is in the TARGET_STATE, add to the count
      if (SW[i].state == TARGET_STATE) count++;
  }

  // if more than one has target state, then print message
  if (count > 1)
  {
    // Print out the switches and reset them as we have 
    // now noted this concurrent event
    Serial.print(F("\nSW "));
    for (uint8_t i = 0; i < ARRAY_SIZE(SW); i++)
    {
      if (SW[i].state == TARGET_STATE)
      {
        SW[i].state = MD_UISwitch::KEY_NULL;
        Serial.print(i);
        Serial.print(F(" "));
      }
    }
    // put in the label
    switch (TARGET_STATE)
    {
    case MD_UISwitch::KEY_NULL:      /*Serial.print(F("KEY_NULL"));*/  break;
    case MD_UISwitch::KEY_UP:        Serial.print(F("KEY_UP"));        break;
    case MD_UISwitch::KEY_DOWN:      Serial.print(F("KEY_DOWN"));      break;
    case MD_UISwitch::KEY_PRESS:     Serial.print(F("KEY_PRESS"));     break;
    case MD_UISwitch::KEY_DPRESS:    Serial.print(F("KEY_DPRESS"));    break;
    case MD_UISwitch::KEY_LONGPRESS: Serial.print(F("KEY_LONGPRESS")); break;
    case MD_UISwitch::KEY_RPTPRESS:  Serial.print(F("KEY_RPTPRESS"));  break;
    default:                         Serial.print(F("KEY_UNKNWN"));    break;
    }
  }
}

void setup(void)
{
  Serial.begin(57600);

  for (uint8_t i = 0; i < ARRAY_SIZE(SW); i++)
  {
    SW[i].sw = new MD_UISwitch_Digital(SW[i].pin, LOW);
    SW[i].sw->begin();
  }
}

void loop(void)
{
  logSwitchState();
  checkConcurrentState();
}
