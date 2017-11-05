#pragma once
/**
\mainpage MD_UISwitch Library

This library uniformly encapsulates the use of different types of switches 
as user input devices.

Switch types handled are:
* Momentary on type switches
* Analog resistor ladder switches
* Keypad matrix

Additional switch types are eaily added through an extensible class model, 
following the existing example code.

The library includes the following features:
* Detects press, double press, long press and auto repeat.
* Works with either either low/high or high/low transitions.
* Software debounce of initial keypress, configurable time period.
* Software double press detection, configurable time period.
* Software long press detection, configurable time period.
* Software auto repeat detection, configurable time period.

See Also
- \subpage pageRevisionHistory
- \subpage pageCopyright

\page pageCopyright Copyright
Copyright
---------
Copyright (C) 2017 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\page pageRevisionHistory Revision History
Revision History
----------------
Nov 2017 version 1.0.0
- New library created to consolidate existing MD_KeySwitch and MD_AButton libraryies
 */

#include <Arduino.h>

/**
 * \file
 * \brief Main header file and class definition for the MD_UISwitch library.
 */

// Default values for timed events.
// Note these are all from the same base (ie when the switch is first detected)
const uint16_t KEY_DEBOUNCE_TIME = 50;    ///< Default key debounce time in milliseconds
const uint16_t KEY_DPRESS_TIME = 250;     ///< Default double press time between presses in milliseconds
const uint16_t KEY_LONGPRESS_TIME = 600;  ///< Default long press detection time in milliseconds
const uint16_t KEY_REPEAT_TIME = 300;     ///< Default time between repeats in in milliseconds
const uint8_t KEY_ACTIVE_STATE = LOW;     ///< Default key is active low - transition high to low detection

// Bit enable/disable
const uint8_t REPEAT_RESULT_ENABLE = 3; ///< Internal status bit to return KS_REPEAT instead of KS_PRESS
const uint8_t DPRESS_ENABLE = 2;        ///< Internal status bit to enable double press
const uint8_t LONGPRESS_ENABLE = 1;     ///< Internal status bit to enable long press
const uint8_t REPEAT_ENABLE = 0;        ///< Internal status bit to enable repeat key

// Miscellaneous defines
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/**
 * Core object for the MD_KeySwitch library
 */
class MD_UISwitch
{
public:
  //--------------------------------------------------------------
  /** \name Enumerated values and Typedefs.
  * @{
  */
  /**
  * Return values for switch status
  *
  * The read() method returns one of these enumerated values as the
  * result of the switch transition detection.
  */
  enum keyResult_t
  {
    KEY_NULL,        ///< No key press
    KEY_PRESS,       ///< Simple press, or a repeated press sequence if enableRepeatResult(false) (default)
    KEY_DPRESS,      ///< Double press
    KEY_LONGPRESS,   ///< Long press
    KEY_RPTPRESS     ///< Repeated key press (only if enableRepeatResult(true))
  };
  /** @} */

  //--------------------------------------------------------------
  /** \name Class constructor and destructor.
  * @{
  */
  /**
   * Class Constructor.
   *
   * Instantiate a new instance of the class. 
   * The main function for the core object is to initialise the internal 
   * shared variables and timers to default values.
   *
   */
  MD_UISwitch(void);

  /**
   * Class Destructor.
   *
   * Release any allocated memory and clean up anything else.
   */
  ~MD_UISwitch() {};
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for core object control.
  * @{
  */
  /**
   * Initialize the object.
   *
   * Initialise the object data. This needs to be called during setup() to initialise new
   * data for the class that cannot be done during the object creation. This method
   * should be replaced in the derived class.
   */
  virtual void begin(void) {};
  
  /**
  * Read input and return the state of the switch
  *
  * Read the input key switch and invoke the process method to determine
  * what the keystroke means. This method needs to be replaced in the derived class
  * with a hardware specific method to read the key switch.
  *
  * \sa processFSM() method
  *
  * \return the keyResult_t enumerated value from processFSM()
  */
  virtual keyResult_t read(void) { return(KEY_NULL); };

  /**
  * Read the key identifier for the last switch
  *
  * Return the id for the last active switch. This is useful to know which key
  * was actually pressed when there could be more than one key (ie, a key matrix).
  *
  * \return an identifiying index for the key item, depending on implementation
  */
  virtual uint8_t getKey(void) { return(_lastKey); };
  
  /**
  * Process the key using FSM
  *
  * Process the key read using a finite state machine to detect the current
  * type of keypress and return one of the keypress types.
  *
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognised by a finite
  * state machine invoked in process() whose operation is directed by the
  * timer and option values specified.
  *
  * \param bState true if the switch is active, false otherwise.
  * \param keyID an optional identifier for the key pressed.
  * \return one of the keyResult_t enumerated values.
  */
  keyResult_t processFSM(bool bState, uint8_t keyID = 0);
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for object parameters and options.
  * @{
  */
  /**
   * Set the debounce time
   *
   * Set the switch debounce time in milliseconds.
   * The default value is set by the KEY_DEBOUNCE_TIME constant.
   *
   * Note that the relationship between timer values should be
   * Debounce time < Long Press Time < Repeat time. No checking 
   * is done in the to enforce this relationship.
   *
   * \param t the specified time in milliseconds.
   */
  inline void setDebounceTime(uint16_t t) 
    { _timeDebounce = t; };

  /**
   * Set the double press detection time
   *
   * Set the time between each press time in milliseconds. A double
   * press is detected if the switch is released and depressed within 
   * this time, measured from when the first press is detected.
   * The default value is set by the KEY_DPRESS_TIME constant.
   *
   * \param t the specified time in milliseconds.
   */
  inline void setDoublePressTime(uint16_t t)
    { _timeDoublePress = t; enableDoublePress(true); };

  /**
   * Set the long press detection time
   *
   * Set the time in milliseconds after which a continuous press 
   * and release is deemed a long press, measured from when the 
   * first press is detected.
   * The default value is set by the KEY_LONGPRESS_TIME constant.
   *
   * Note that the relationship betweentimer values should be
   * Debounce time < Long Press Time < Repeat time. No checking 
   * is done in the to enforce this relationship.
   *
   * \param t the specified time in milliseconds.
   */
  inline void setLongPressTime(uint16_t t)
    { _timeLongPress = t; enableLongPress(true); };

  /**
   * Set the repeat time
   *
   * Set the time in milliseconds after which a continuous press
   * and hold is treated as a stream of repeated presses, measured
   * from when the first press is detected.
   *
   * Note that the relationship between timer values should be
   * Debounce time < Long Press Time < Repeat time. No checking 
   * is done in the to enforce this relationship.
   *
   * \param t the specified time in milliseconds.
   */
  inline void setRepeatTime(uint16_t t)
    { _timeRepeat = t; enableRepeat(true); };

  /**
   * Enable double press detection
   *
   * Enable or disable double press detection. If disabled,
   * two single press are detected instead of a double press.
   * Default is to detect double press events.
   *
   * \param f true to enable, false to disable.
   */
  inline void enableDoublePress(boolean f)
    { if (f) bitSet(_enableFlags, DPRESS_ENABLE); else bitClear(_enableFlags, DPRESS_ENABLE); };

  /**
   * Enable long press detection
   *
   * Enable or disable long press detection. If disabled,
   * the long press notification is skipped when the event 
   * is detected and either a simple press or repeats are 
   * returned, depening on the setting of the other options.
   * Default is to detect long press events.
   *
   * \param f true to enable, false to disable.
   */
  inline void enableLongPress(boolean f)
    { if (f) bitSet(_enableFlags, LONGPRESS_ENABLE); else bitClear(_enableFlags, LONGPRESS_ENABLE); };

  /**
   * Enable repeat detection
   *
   * Enable or disable repeat detection. If disabled,
   * the long press notification is returned as soon as the
   * long press time has expired.
   * Default is to detect repeat events.
   *
   * \param f true to enable, false to disable.
   */
  inline void enableRepeat(boolean f)
    { if (f) bitSet(_enableFlags, REPEAT_ENABLE); else bitClear(_enableFlags, REPEAT_ENABLE); };

  /**
   * Modify repeat notification
   *
   * Modify the result returned from a repeat detection. 
   * If enabled, the first repeat will return a KS_PRESS and
   * subsequent repeats will return KS_RPTPRESS. If disabled 
   *(default) the repeats will be stream of KS_PRESS values.
   *
   * \param f true to enable, false to disable (default).
   */
  inline void enableRepeatResult(boolean f)
    { if (f) bitSet(_enableFlags, REPEAT_RESULT_ENABLE); else bitClear(_enableFlags, REPEAT_RESULT_ENABLE); };
  /** @} */

protected:
  /**
  * FSM state values
  *
  * States for the internal Finite State Machine to recognised the key press
  */
  enum state_t 
  { 
    S_IDLE,       ///< Idle state - waiting for key transition
    S_DEBOUNCE1,  ///< First press debounce 
    S_DEBOUNCE2,  ///< Second (double) press debounce
    S_PRESS,      ///< Detecting possible simple press
    S_DPRESS,     ///< Detecting possible double press
    S_LPRESS,     ///< Detecting possible long press
    S_REPEAT,     ///< Outputting repeat keys when timer expires
    S_WAIT        ///< Waiting for key to be released after long press is detected
  };

  state_t   _state;       ///< the FSM current state
  uint32_t  _timeActive;  ///< the millis() time it was last activated
  uint8_t   _enableFlags; ///< functions enabled/disabled

  // Note that Debounce time < Long Press Time < Repeat time. No checking is done in the
  // library to enforce this relationship.
  uint16_t  _timeDebounce;  ///< debounce time in milliseconds
  uint16_t  _timeDoublePress; ///< double press detection time in milliseconds
  uint16_t  _timeLongPress; ///< long press time in milliseconds
  int16_t   _timeRepeat;    ///< repeat time delay in milliseconds
  
  uint8_t   _lastKey;       ///< persists the last key value until a new one shows up
};

/**
* Extension object for MD_UISwitch_Digital, implementing momentary ON switches
*/
class MD_UISwitch_Digital: public MD_UISwitch
{
public:
  //--------------------------------------------------------------
  /** \name Class constructor and destructor.
  * @{
  */
  /**
  * Class Constructor.
  *
  * Instantiate a new instance of the class. The parameters passed are
  * used to the hardware interface to the switch.
  *
  * The option parameter onState tells the library which level
  * (LOW or HIGH) should be considered the switch 'on' state. If the
  * default LOW state is selected then the library will initialise the
  * pin with INPUT_PULLUP and no external pullup resistors are necessary.
  * If specified HIGH, external pull down resistors will be required.
  *
  * \param pin   the digital pin to which the switch is connected.
  * \param onState   the state for the switch to be active
  */
  MD_UISwitch_Digital(uint8_t pin, uint8_t onState = KEY_ACTIVE_STATE) :
  _pin(pin), _onState(onState) {};

  /**
  * Class Destructor.
  *
  * Release allocated memory and does the necessary to clean up once the queue is
  * no longer required.
  */
  ~MD_UISwitch_Digital() {};

  /** @} */
  //--------------------------------------------------------------
  /** \name Methods for core object control.
  * @{
  */
  /**
  * Initialize the object.
  *
  * Initialise the object data. This needs to be called during setup() to initialise new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the state of the switch
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognised by a finite
  * state machine whose operation is directed by the timer and option
  * values specified.
  *
  * \return one of the keyResult_t enumerated values
  */
  virtual keyResult_t read(void);
  /** @} */

protected:
  uint8_t   _pin;       ///< pin number
  uint8_t   _onState;   ///< digital state for ON
};

/**
* Extension object for MD_UISwitch_Analog, implementing resistor ladder switches
*/
class MD_UISwitch_Analog : public MD_UISwitch
{
public:
  //--------------------------------------------------------------
  /** \name Enumerated values and Typedefs.
  * @{
  */
  /**
  * Data table for switch values
  *
  * The read() will use this table to determine which key has been pressed
  * and getKey() will return the specified identifier.
  */
  typedef struct
  {
    uint16_t  adcThreshold; ///< Average analog value of the 
    uint8_t   adcTolerance; ///< Valid range for a key will be adcThreshold +/- Tolerance
    uint8_t   value;        ///< Identifier for this key, returned using getKey()
  } uiAnalogKeys_t;
  /** @} */

  //--------------------------------------------------------------
  /** \name Class constructor and destructor.
  * @{
  */
  /**
  * Class Constructor.
  *
  * Instantiate a new instance of the class. The parameters passed are
  * used to the hardware interface to the switch.
  *
  * \param pin    the analog pin to which the switches are connected.
  * \param kt     pointer to a table of analog value key definitions
  * \param ktSize number of elements in the kt table
  */
  MD_UISwitch_Analog(uint8_t pin, uiAnalogKeys_t* kt, uint8_t ktSize) :
    _pin(pin), _kt(kt), _ktSize(ktSize), _lastKeyIdx(-1) {};

  /**
  * Class Destructor.
  *
  * Release allocated memory and does the necessary to clean up once the queue is
  * no longer required.
  */
  ~MD_UISwitch_Analog() {};
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for core object control.
  * @{
  */
  /**
  * Initialize the object.
  *
  * Initialise the object data. This needs to be called during setup() to initialise new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the state of the switch
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognised by a finite
  * state machine whose operation is directed by the timer and option
  * values specified.
  *
  * \return one of the keyResult_t enumerated values
  */
  virtual keyResult_t read(void);
  /** @} */

protected:
  uint8_t     _pin;     ///< pin number
  uiAnalogKeys_t* _kt;  ///< analog key values table
  uint8_t   _ktSize;    ///< umber of elements in analog keys table

  int16_t   _lastKeyIdx;  ///< index of the last key read
};


/**
* Extension object for MD_UISwitch_Matrix, implementing keyboard matrix switches
*/
class MD_UISwitch_Matrix : public MD_UISwitch
{
public:
  //--------------------------------------------------------------
  /** \name Class constructor and destructor.
  * @{
  */
  /**
  * Class Constructor.
  *
  * Instantiate a new instance of the class. The parameters passed are
  * used to the hardware interface to the switch.
  *
  * \param rows    the number of rows in the key matrix.
  * \param cols    the number of columns in the key matrix.
  * \param rowPin  array of rows elements of ordered row pins to which the switches are connected.
  * \param colPin  array of cols elements of ordered column pins to which the switches are connected.
  * \param kt      the key table. An array of characters arranged in by column then row. 
  */
  MD_UISwitch_Matrix(uint8_t rows, uint8_t cols, uint8_t* rowPin, uint8_t* colPin, char* kt) :
    _rows(rows), _cols(cols), _rowPin(rowPin), _colPin(colPin), _kt(kt) {};

  /**
  * Class Destructor.
  *
  * Release allocated memory and does the necessary to clean up once the queue is
  * no longer required.
  */
  ~MD_UISwitch_Matrix() {};
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for core object control.
  * @{
  */
  /**
  * Initialize the object.
  *
  * Initialise the object data. This needs to be called during setup() to initialise new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the state of the switch
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognised by a finite
  * state machine whose operation is directed by the timer and option
  * values specified.
  *
  * \return one of the keyResult_t enumerated values
  */
  virtual keyResult_t read(void);
  /** @} */

protected:
  uint8_t   _rows;       ///< number of rows in the key matrix
  uint8_t   _cols;       ///< number of columns in the key matrix
  uint8_t   *_rowPin;    ///< array of pins connected to the rows 
  uint8_t   *_colPin;    ///< array of pins connected to the columns
  char      *_kt;        ///< analog key values in a char string

  int16_t   _lastKeyIdx;  ///< index of the last key read
};
