/*
MD_UISwitch class implementation.

See main header file for information.
*/

#include "MD_UISwitch.h"

const int16_t KEY_IDX_UNDEF = -1;   ///< Index value for undefined index

/**
 * \file
 * \brief Main code file for MD_UISwitch library
 */

#define UI_DEBUG 0  ///< Library debugging flag

#if UI_DEBUG
#define UI_PRINTS(s)   do { Serial.print(F(s)); } while (false)                  ///< Debugging macro
#define UI_PRINT(s, v) do { Serial.print(F(s)); Serial.print(v); } while (false) ///< Debugging macro
#else
#define UI_PRINTS(s)    ///< Debugging macro
#define UI_PRINT(s, v)  ///< Debugging macro
#endif

MD_UISwitch::MD_UISwitch(void) : _state(S_IDLE), _lastKeyIdx(KEY_IDX_UNDEF)
{
  setPressTime(KEY_PRESS_TIME);
  setDoublePressTime(KEY_DPRESS_TIME);
  setLongPressTime(KEY_LONGPRESS_TIME);
  setRepeatTime(KEY_REPEAT_TIME);
  enableRepeatResult(false);
  debounce(false, true);    // reset the debounce routine
  processFSM(false, true);  // reset the FSM
}

bool MD_UISwitch::debounce(bool curStatus, bool reset)
/*
  Switch debounce using Edge Detection & Resistor-Capacitor Digital Filter.

  This Digital filter mimics an analogue RC filter with first-order
  recursive low pass filter. It has good EMI filtering and quick response,
  with a nearly continuous output like an analogue circuit.

  Based on Elio Mazzocca, Contact-debouncing algorithm emulates Schmitt trigger.
  Based on Oregon State University, Debounce switches algorithm.
  Based on John Youngquist, Debouncing Switches and Encoders.
  Version by Andrew M. Kollosche, 2015-2018 at http://www.ganssle.com/tem/tem366.html

  Time-Constant  TCSHIFT, TC,  UTH,  LTH, Approx execute times on a PIC16 @ 2MHz.
  Quarter           2,    63,  242,   15,  1ms.
  Eight             3,    31,  238,   12,  2ms.
  Sixteenth         4,    15,  230,   10,  3ms.
  Thirty-second     5,     7,  215,    7,  9ms.
  Sixty-forth       6,     3,  184,    3,  15ms.

  Note: Thresholds count for each RC time-constant,
  is calculated on 3 time-constants or for Upper threshold is 96% and
  3% for Lower Threshold.
*/
{
  // --- These constants are a consistent set from the table above
  const uint8_t TC_SHIFT = 5;   // bit shift for fraction
  const uint8_t TC = 7;         // RC time constant
  const uint8_t UTH = 215;      // Upper Threshold  for 'on'
  const uint8_t LTH = 7;        // Lower Threshold for 'off'
  // ---

  // handle the reset
  if (reset)
  {
    _RC = 0;
    _prevStatus = false;
    _RCstate = S_WAIT_START;
  }

  bool b = _prevStatus; // return status value

  //edge detector from 'inactive' to 'active'
  switch (_RCstate)
  {
    case S_WAIT_START: // wait for 'inactive' to 'active' transition
    {
      if (!_prevStatus && curStatus)
        _RCstate = S_DEBOUNCE;
      _prevStatus = curStatus;
    }
    break;

    case S_DEBOUNCE:  // RC debounce
    {
      //first compute RCnew = RCold * fraction. Using shift and subtraction.
      uint8_t temp = _RC >> TC_SHIFT;
      _RC = _RC - temp;   // subtract fraction for result.

      if (curStatus)    // still an active switch
        _RC += TC;       // add in the time constant
      else
        if (_RC > 0) _RC--; // 'leak' and decay the value

      //check upper & lower threshold.
      b = (_RC > UTH);     // upper threshold means we have a valid result
      if (b or _RC < LTH)  // got result or lower threshold
      {
        _RC = 0;               // reset RC value
        _RCstate = S_WAIT_RELEASE; // move on to next state
      }
    }
    break;

    case S_WAIT_RELEASE:  // waiting for switch release
    default:
    {
      if (!curStatus) _RCstate = S_WAIT_START;
    }
    break;
  }

  // UIPRINTS((b) ? "-> DOWN" : "<- UP");

  return (b);
}

MD_UISwitch::keyResult_t MD_UISwitch::processFSM(bool b, bool reset)
// Return one of the keypress types depending on what has been detected
// in the FSM logic
{
  keyResult_t k = KEY_NULL;

  if (reset)
  {
    _state = S_IDLE;
    _kPush = KEY_NULL;
    return(k);
  }

  // If we have previously pushed something return that status now
  if (_kPush != KEY_NULL)
  {
    k = _kPush;
    _kPush = KEY_NULL;
    return(k);
  }

  // Now run the FSM with the input
  switch (_state)
  {
  case S_IDLE:    // waiting for first transition
    //UI_PRINT("\nS_IDLE", b);
    if (b)
    {
      _state = S_PRESS;
      _timeActive = millis();
      k = KEY_DOWN;
    }
    break;

  case S_PRESS:   // press?
    UI_PRINT("\nS_PRESS ", b);
    // Key off before a long press registered, so it is either double press or a press.
    if (!b)
    {
      k = KEY_UP;
      if (bitRead(_enableFlags, DPRESS_ENABLE))  // DPRESS allowed
      {
        _state = S_PRESS2A;
        _timeActive = millis();
      }
      else      // this is just a press
      {
        _kPush = KEY_PRESS;
        _state = S_IDLE;
      }
    }

    // if the switch is still on and we have run out of press time ...
    if (millis() - _timeActive > _timePress)
    {
      _timeActive = millis();   // reset for repeat timer base
      // ... we either have a long press or are 
      // heading towards repeats if they are enabled
      if (bitRead(_enableFlags, LONGPRESS_ENABLE)) 
        _state = S_PRESSL;
      else if (bitRead(_enableFlags, REPEAT_ENABLE))
      {
        k = KEY_PRESS;
        _state = S_REPEAT;
      }
      else // nothing else that can be done as we have no time left!
      {
        k = KEY_PRESS;
        _state = S_WAIT;
      }
    }
    break;

  case S_PRESSL:  // long press or auto repeat?
    UI_PRINT("\nS_PRESSL ", b);
    // It is a long press if
    // - Key off before a repeat press is registered, or
    // - Auto repeat is disabled
    // Set the return code and go back to waiting
    if (!b)
    {
      _kPush = KEY_LONGPRESS;
      k = KEY_UP;
      _state = S_IDLE;
      break;
    }

    if (millis() - _timeActive > _timeLongPress)
    {
      if (bitRead(_enableFlags, REPEAT_ENABLE))
      {
        k = KEY_PRESS;      // the first of the repeats
        _state = S_REPEAT;  // handle the rest of them
        _timeActive = millis();  // set the new baseline time.
      }
      else  // no repeats - register the long press and wait for release
      {
        k = KEY_LONGPRESS;
        _state = S_WAIT;
      }
    }
    break;

  case S_REPEAT: // repeat?
    UI_PRINT("\nS_REPEAT ", b);
    // Key off before another repeat press is registered, so we have finished.
    // Go back to waiting
    if (!b)
    {
      k = KEY_UP;
      _state = S_IDLE;
    }
    else    // if (b)
    {
      // if the switch is still on and we have not run out of repeat time, then
      // just wait for the timer to expire.
      if ((millis() - _timeActive) < _timeRepeat)
        break;

      // we are now sure we have a repeat, set the return code and remain in this
      // state checking for further repeats if enabled
      k = bitRead(_enableFlags, REPEAT_RESULT_ENABLE) ? KEY_RPTPRESS : KEY_PRESS;
      _timeActive = millis();	// next key repeat time starts now
    }
    break;

  case S_PRESS2A:   // Wait for key to be pressed again in double press sequence
    UI_PRINT("\nS_PRESS2A ", b);
    if (b)
    {
      k = KEY_DOWN;
      _state = S_PRESS2B;		// switch detected, initiate second
      _timeActive = millis();
    }

    // Check if we didn't get a second press within time - 
    // then this was just a press and wait for key release
    if (millis() - _timeActive  > _timeDoublePress)
    {
      k = KEY_PRESS;
      _state = (b) ? S_WAIT : S_IDLE;
    }
    break;

  case S_PRESS2B:   // Wait for key to be released in double press sequence
    UI_PRINT("\nS_PRESS2B ", b);
    if (!b)
    {
      k = KEY_UP;
      _kPush = KEY_DPRESS;
      _state = S_IDLE;
    }

    // we didn't get a second release within time then this was just a press
    // and we wait for the key to be released
    if (millis() - _timeActive >= _timePress*2)
    {
      _kPush = KEY_PRESS;
      _state = (b) ? S_WAIT : S_IDLE;
    }
    break;

  case S_WAIT:
  default:
    // After completing while still key active, allow the user to release the switch
    // to meet starting conditions for S_IDLE
    UI_PRINT("\nS_WAITING ", b);
    if (!b)
    {
      k = KEY_UP;
      _state = S_IDLE;
    }
    break;
  }

  return(k);
}


// -----------------------------------------------
// MD_UISwitch_Digital methods
// -----------------------------------------------
void MD_UISwitch_Digital::begin(void)
{
  UI_PRINT("\nUISwitch_Digital begin() ", _pinCount);
  UI_PRINTS(" pins");

  for (uint8_t i = 0; i < _pinCount; i++)
    pinMode(_pins[i], _onState == LOW ? INPUT_PULLUP : INPUT);
}

MD_UISwitch::keyResult_t MD_UISwitch_Digital::read(void)
{
  bool b = false;
  int16_t idx = KEY_IDX_UNDEF;
  int16_t count = 0;

  // work out which key is pressed
  for (uint8_t i = 0; i < _pinCount; i++)
  {
    if (digitalRead(_pins[i]) == _onState)
    {
      if (idx == KEY_IDX_UNDEF) idx = i;  // only record the first one
      count++;
    }
  }

  // if more than one key pressed, don't count anything
  if (count == 1)   // we have a valid key
  {
    // is this the same as the previous key?
    if (idx != _lastKeyIdx) processFSM(debounce(false, true), true); // reset the debounce and FSM

    b = (idx == _lastKeyIdx);
    _lastKeyIdx = idx;
    _lastKey = _pins[idx];
    //UI_PRINT("\nKey idx ", _lastKeyIdx);
    //UI_PRINT(" value ", _lastKey);
  }

  return(processFSM(debounce(b)));
}
// -----------------------------------------------

// -----------------------------------------------
// MD_UISwitch_User methods
// -----------------------------------------------
void MD_UISwitch_User::begin(void)
{
  UI_PRINT("\nUISwitch_User begin() ", _idCount);
  UI_PRINTS(" ids");
}

MD_UISwitch::keyResult_t MD_UISwitch_User::read(void)
{
  bool b = false;
  int16_t idx = KEY_IDX_UNDEF;
  int16_t count = 0;

  // work out which key is pressed
  for (uint8_t i = 0; i < _idCount; i++)
  {
    if (_cb(_ids[i]))
    {
      if (idx == KEY_IDX_UNDEF) idx = i;  // only record the first one
      count++;
    }
  }

  // if more than one key pressed, don't count anything
  if (count == 1)   // we have a valid key
  {
    // is this the same as the previous key?
    if (idx != _lastKeyIdx) processFSM(debounce(false, true), true); // reset the debounce and FSM

    b = (idx == _lastKeyIdx);
    _lastKeyIdx = idx;
    _lastKey = _ids[idx];
    //UI_PRINT("\nKey idx ", _lastKeyIdx);
    //UI_PRINT(" value ", _lastKey);
  }

  return(processFSM(debounce(b)));
}
// -----------------------------------------------

// -----------------------------------------------
// MD_UISwitch_Analog methods
// -----------------------------------------------
void MD_UISwitch_Analog::begin(void)
{
  UI_PRINTS("\nUISwitch_Analog begin()");
  pinMode(_pin, INPUT);
}

MD_UISwitch::keyResult_t MD_UISwitch_Analog::read(void)
{
  bool b = false;
  uint16_t v = analogRead(_pin);
  int16_t idx = KEY_IDX_UNDEF;

  // work out what key this is
  for (uint8_t i = 0; i < _ktSize; i++)
  {
    if ((v >= (_kt[i].adcThreshold - _kt[i].adcTolerance)) &&
        (v <= (_kt[i].adcThreshold + _kt[i].adcTolerance)))
    {
      idx = i;
      break;
    }
  }

  // is this a valid key the same as the previous key?
  if (idx != KEY_IDX_UNDEF)
  {
    // is this the same as the previous key?
    if (idx != _lastKeyIdx) processFSM(debounce(false, true), true); // reset the FSM

    b = (idx == _lastKeyIdx);
    _lastKeyIdx = idx;
    _lastKey = _kt[idx].value;
    UI_PRINT("\nKey idx ", _lastKeyIdx);
    UI_PRINT(" value ", _lastKey);
  }

  return(processFSM(debounce(b)));
}
// -----------------------------------------------

// -----------------------------------------------
// MD_UISwitch_Matrix methods
// -----------------------------------------------
void MD_UISwitch_Matrix::begin(void)
{
  UI_PRINTS("\nUISwitch_Matrix begin()");

  // initialize the hardware
  for (uint8_t i = 0; i < _rows; i++) 
    pinMode(_rowPin[i], INPUT_PULLUP);

  // columns initialize during scanning
  //for (uint8_t i = 0; i < _rows; i++)
  //  pinMode(_colPin[i], OUTPUT);
}

MD_UISwitch::keyResult_t MD_UISwitch_Matrix::read(void)
{
  bool b = false;
  int16_t idx = KEY_IDX_UNDEF;
  int16_t count = 0;

  // scan the keypad and stop at the first key detected
  for (uint8_t c = 0; c < _cols; c++) 
  {
    pinMode(_colPin[c], OUTPUT);
    digitalWrite(_colPin[c], LOW);	    // column pulse
    for (uint8_t r = 0; r < _rows; r++)
    {
      if (digitalRead(_rowPin[r]) == LOW)
      {
        if (idx == KEY_IDX_UNDEF) idx = (r * _cols) + c;
        count++;
        UI_PRINT("\nC:", c);
        UI_PRINT(" R:", r);
        UI_PRINT(" idx:", idx);
      }
    }
    digitalWrite(_colPin[c], HIGH);     // end column pulse
    pinMode(_colPin[c], INPUT);         // set high impedance
  }

  // if more than one key pressed, don't count anything
  if (count == 1)  // we have a valid key
  {
    // is this the same as the previous key?
    if (idx != _lastKeyIdx) processFSM(debounce(false, true), true); // reset the FSM

    b = (idx == _lastKeyIdx);
    _lastKeyIdx = idx;
    _lastKey = _kt[idx];
    UI_PRINT("\nKey idx ", _lastKey);
    UI_PRINT(" value ", _lastKey);
  }

  return(processFSM(debounce(b)));
}
// -----------------------------------------------

// -----------------------------------------------
// MD_UISwitch_4017KM methods
// -----------------------------------------------
void MD_UISwitch_4017KM::begin(void)
{
  UI_PRINTS("\nUISwitch_4017KM begin()");

  // initialize the hardware
  digitalWrite(_pinClk, LOW);
  pinMode(_pinClk, OUTPUT);
  pinMode(_pinKey, INPUT);
  if (_pinRst != 0)
  {
    pinMode(_pinRst, OUTPUT);
    digitalWrite(_pinRst, LOW);
  }
}

void MD_UISwitch_4017KM::reset(void)
{
  digitalWrite(_pinRst, HIGH);
  delayMicroseconds(1);
  digitalWrite(_pinRst, LOW);
}

void MD_UISwitch_4017KM::clock(void)
{
  digitalWrite(_pinClk, HIGH);
  // delayMicroseconds(1); // may not be needed!
  digitalWrite(_pinClk, LOW);
}

MD_UISwitch::keyResult_t MD_UISwitch_4017KM::read(void)
{
  bool b = false;
  int16_t idx = KEY_IDX_UNDEF;
  int16_t count = 0;

  reset();

  // scan the keypad and stop at the first key detected
  for (int16_t i = 0; i<_numKeys; i++)
  {
    // read and advance the counter	
    if (digitalRead(_pinKey) == HIGH)
    {
      if (idx == KEY_IDX_UNDEF) idx = i;
      count++;
    }
    clock();    // advance the 4017 counter
  }

  // if more than one key pressed, don't count anything
  if (count == 1)  // we have a valid key
  {
    // is this the same as the previous key?
    if (idx != _lastKey) processFSM(debounce(false, true), true); // reset the FSM

    b = (idx == _lastKeyIdx);
    _lastKeyIdx = idx;
    _lastKey = idx;
    UI_PRINT("\nKey idx ", _lastKey);
    UI_PRINT(" value ", _lastKey);
  }

  return(processFSM(debounce(b)));
}
// -----------------------------------------------
