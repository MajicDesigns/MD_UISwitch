#pragma once
/**
\mainpage MD_UISwitch Library

This library uniformly encapsulates the use of different types of switches 
for user input devices. The library is easily extended for additional 
switch types through a class hierarchy and inheritance model, following 
the code for existing switch types.

The library includes the following features:
- Software debounce for all switch types.
- Automatically detect switch press, double press, long press and auto repeat.
- Can work with low/high or high/low transitions.
- All software timers are configurable for fine tuning to specific applications. These include:
 + debounce time.
 + double press time.
 + long press time.
 + auto repeat period time.

Switch arrangements handled by the library are:
- Momentary on type switches (MD_Switch_Digital class)
- Analog resistor ladder switches (MD_Switch_Analog class)
- Keypad matrix (MD_Switch_Matrix class)
- Keypad matrix using 4017 IC (MD_Matrix_4017KM class)

See Also
- \subpage pageRevisionHistory
- \subpage pageCopyright
- \subpage pageDonation

\page pageDonation Support the Library
If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)

\page pageCopyright Copyright
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
Apr 2019 version 2.1.0
- Fixed bugs caused by statics in functions (bad mistake!)
- Added MultiDigital example

Mar 2019 version 2.0.0
- Added separate debounce rountine
- Added Key-UP and KEY_DOWN indicators
- Replaced setDebounceTime() with setPressTime()
- Simplified FSM to eliminate FSM debounce code

Dec 2017 version 1.2.0
- Changed Digital readKey() value to be the pin number not array index

Nov 2017 version 1.1.0
- Corrected keywords.txt
- Added array of pins constructor to UISwitch_Digital
- Corrected some switch detection code

Nov 2017 version 1.0.0
- New library created to consolidate existing MD_KeySwitch and MD_AButton libraries
 */

#include <Arduino.h>

/**
 * \file
 * \brief Main header file and class definition for the MD_UISwitch library.
 */

// Default values for timed events.
// Note these are all from the same base (ie when the switch is first detected)
const uint16_t KEY_PRESS_TIME = 150;      ///< Default key press time in milliseconds
const uint16_t KEY_DPRESS_TIME = 250;     ///< Default double press time between presses in milliseconds
const uint16_t KEY_LONGPRESS_TIME = 600;  ///< Default long press detection time in milliseconds
const uint16_t KEY_REPEAT_TIME = 300;     ///< Default time between repeats in in milliseconds
const uint8_t  KEY_ACTIVE_STATE = LOW;    ///< Default key is active low - transition high to low detection

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
    KEY_DOWN,        ///< Switch is detected as active (down)
    KEY_UP,          ///< Switch is detected as inactive (up)
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
   * The main function for the core object is to initialize the internal 
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
   * Initialize the object data. This needs to be called during setup() to initialize new
   * data for the class that cannot be done during the object creation. This method
   * must be replaced in the derived class.
   */
  virtual void begin(void) = 0;
  
  /**
  * Read input and return the state of the switch
  *
  * Read the input key switch and invoke the process method to determine
  * what the keystroke means. This method must be replaced in the derived class
  * with a hardware specific method to read the key switch.
  *
  * \sa processFSM() method
  *
  * \return the keyResult_t enumerated value from processFSM()
  */
  virtual keyResult_t read(void) = 0;

  /**
  * Read the key identifier for the last switch
  *
  * Return the id for the last active switch. This is useful to know which key
  * was actually pressed when there could be more than one key (ie, a key matrix).
  *
  * \return an identifying index for the key item, depending on implementation
  */
  virtual uint8_t getKey(void) { return(_lastKey); };
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for object parameters and options.
  * @{
  */
  /**
  * Set the press time
  *
  * Set the switch press time in milliseconds.
  * The default value is set by the KEY_PRESS_TIME constant.
  *
  * Note that the relationship between timer values should be
  * Press Time < Long Press Time < Repeat time. No checking
  * is done in the to enforce this relationship.
  *
  * \param t the specified time in milliseconds.
  */
  inline void setPressTime(uint16_t t) { _timePress = t; };

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
  inline void setDoublePressTime(uint16_t t) { _timeDoublePress = t; enableDoublePress(true); };

  /**
   * Set the long press detection time
   *
   * Set the time in milliseconds after which a continuous press 
   * and release is deemed a long press, measured from when the 
   * first press is detected.
   * The default value is set by the KEY_LONGPRESS_TIME constant.
   *
   * Note that the relationship between timer values should be
   * Press Time < Long Press Time < Repeat time. No checking 
   * is done in the to enforce this relationship.
   *
   * \param t the specified time in milliseconds.
   */
  inline void setLongPressTime(uint16_t t) { _timeLongPress = t; enableLongPress(true); };

  /**
   * Set the repeat time
   *
   * Set the time in milliseconds after which a continuous press
   * and hold is treated as a stream of repeated presses, measured
   * from when the first press is detected.
   *
   * Note that the relationship between timer values should be
   * Press Time < Long Press Time < Repeat time. No checking 
   * is done in the to enforce this relationship.
   *
   * \param t the specified time in milliseconds.
   */
  inline void setRepeatTime(uint16_t t) { _timeRepeat = t; enableRepeat(true); };

  /**
   * Enable double press detection
   *
   * Enable or disable double press detection. If disabled,
   * two single press are detected instead of a double press.
   * Default is to detect double press events.
   *
   * \param f true to enable, false to disable.
   */
  inline void enableDoublePress(boolean f) { (f) ? bitSet(_enableFlags, DPRESS_ENABLE) : bitClear(_enableFlags, DPRESS_ENABLE); };

  /**
   * Enable long press detection
   *
   * Enable or disable long press detection. If disabled,
   * the long press notification is skipped when the event 
   * is detected and either a simple press or repeats are 
   * returned, depending on the setting of the other options.
   * Default is to detect long press events.
   *
   * \param f true to enable, false to disable.
   */
  inline void enableLongPress(boolean f) { (f) ? bitSet(_enableFlags, LONGPRESS_ENABLE) : bitClear(_enableFlags, LONGPRESS_ENABLE); };

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
  inline void enableRepeat(boolean f) { (f) ? bitSet(_enableFlags, REPEAT_ENABLE) : bitClear(_enableFlags, REPEAT_ENABLE); };

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
  inline void enableRepeatResult(boolean f) { (f) ? bitSet(_enableFlags, REPEAT_RESULT_ENABLE) : bitClear(_enableFlags, REPEAT_RESULT_ENABLE); };
  /** @} */

protected:
  /**
  * FSM state values
  *
  * States for the internal Finite State Machine to recognized the key press
  */
  enum state_fsm 
  { 
    S_IDLE,       ///< Idle state - waiting for key transition
    S_PRESS,      ///< Detecting possible simple press
    S_PRESS2A,    ///< Detecting possible double press part A
    S_PRESS2B,    ///< Detecting possible double press part B
    S_PRESSL,     ///< Detecting possible long press
    S_REPEAT,     ///< Outputting repeat keys when timer expires
    S_WAIT        ///< Waiting for key to be released after long press is detected
  };

  // FSM persistent values
  state_fsm _state;       ///< the FSM current state
  keyResult_t _kPush;     ///< storage for pushed key in FSM
  uint32_t  _timeActive;  ///< the millis() time switch was last activated
  uint8_t   _enableFlags; ///< functions enabled/disabled

  /**
  * Debouncing state values
  *
  * States for the internal Debouncing algorithm
  */
  enum state_db
  { 
    S_WAIT_START,   ///< Waiting for the debouncing to start
    S_DEBOUNCE,     ///< Currently debouncing state
    S_WAIT_RELEASE  ///< Waiting for the next transition to be detected
  };

  // Debouncing persistent values
  uint8_t _RC = 0;    ///< RC integrator value
  bool _prevStatus;   ///< previous 'active' status for edge detection
  state_db _RCstate;  ///< current RC debouning state


  // Note that Press time < Long Press Time < Repeat time. No checking is done in the
  // library to enforce this relationship.
  uint16_t  _timePress;     ///< press time in milliseconds
  uint16_t  _timeDoublePress; ///< double press detection time in milliseconds
  uint16_t  _timeLongPress; ///< long press time in milliseconds
  uint16_t  _timeRepeat;    ///< repeat time delay in milliseconds
  
  uint8_t   _lastKey;       ///< persists the last key value until a new one is detected
  int16_t   _lastKeyIdx;    ///< internal index of the last key read

  /**
  * Process the key using FSM
  *
  * Process the key read using a finite state machine to detect the current
  * type of keypress and return one of the keypress types.
  *
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognized by a finite
  * state machine invoked in process() whose operation is directed by the
  * timer and option values specified.
  *
  * If the reset parameter is specified the FSM is reset to the idle state
  * and no other processing is performed (ie, the bState parameter is
  * ignored).
  *
  * \param swState true if the switch is active, false otherwise.
  * \param reset  an optional identifier to reset the FSM.
  * \return one of the keyResult_t enumerated values.
  */
  keyResult_t processFSM(bool swState, bool reset = false);

  /**
  * Switch debounce using Edge Detection & Resistor-Capacitor Digital Filter.
  *
  * A digital filter that mimics an analogue RC filter with first-order
  * recursive low pass filter. It has good EMI filtering and quick response,
  * with a nearly continuous output like an analogue circuit.
  *
  * \param curStatus  current active status for the switch.
  * \param reset  an optional identifier to reset the debounce detection.
  * \return true if the switch is 'debounced' active, false otherwise.
  */
  bool debounce(bool curStatus, bool reset = false);
};

/**
* Extension class MD_UISwitch_Digital.
*
* Implements interface for momentary ON switches, such as tact switches or micro-switches.
*
* ![Momentary On Digital Style Switches] (Digital_Switches.jpg "Digital Switches")
*
* Switches can be wired as pull-up or pull-down. Pull-up switches are initialized 
* with the internal pull-up enabled. Pull-down switches require an external 
* pull-down resistor circuit. How the switch type is initialized depends on the
* parameters passed to the class constructor.
*/
class MD_UISwitch_Digital: public MD_UISwitch
{
public:
  //--------------------------------------------------------------
  /** \name Class constructor and destructor.
  * @{
  */
  /**
  * Class Constructor - simple pin.
  *
  * Instantiate a new instance of the class. The parameters passed are
  * used to the hardware interface to the switch.
  *
  * This form of the constructor is for a simple digital pin (ie,
  * one digital pin). 
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
    _pinSimple(pin), _pins(&_pinSimple), _pinCount(1), _onState(onState) {};

  /**
  * Class Constructor - array of pins.
  *
  * Instantiate a new instance of the class. The parameters passed are
  * used to the hardware interface to the switch.
  *
  * This form of the constructor is for an array of digital pins. The
  * data is not copied from the user code, so the array elements need
  * to remain in scope and constant for the life of the object.
  *
  * The option parameter onState tells the library which level
  * (LOW or HIGH) should be considered the switch 'on' state. If the
  * default LOW state is selected then the library will initialize each
  * pin with INPUT_PULLUP and no external pullup resistors are necessary.
  * If specified HIGH, external pull down resistors will be required.
  *
  * \param pins      pointer to array of pin numbers to which the switches are connected.
  * \param pinCount  the number of pin in the pins[] array
  * \param onState   the state for the switch to be active
  */
  MD_UISwitch_Digital(uint8_t *pins, uint8_t pinCount, uint8_t onState = KEY_ACTIVE_STATE) :
    _pins(pins), _pinCount(pinCount), _onState(onState) {};

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
  * Initialise the object data. This needs to be called during setup() to initialize new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the state of the switch
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognized by a finite
  * state machine whose operation is directed by the timer and option
  * values specified.
  *
  * \return one of the keyResult_t enumerated values
  */
  virtual keyResult_t read(void);
  /** @} */

protected:
  uint8_t   _pinSimple; ///< pin number for simple pins
  uint8_t   *_pins;     ///< pointer to data for one or more pins
  uint8_t   _pinCount;  ///< number of pins defined
  uint8_t   _onState;   ///< digital state for ON
};

/**
* Extension class MD_UISwitch_Analog.
*
* Implements resistor ladder switches on analog input. This type of switch
* are typically found on 1602 LCD module shields.
*
* ![LCD Shield Analog Style Switches] (lcd_shield.jpg "LCD Shield")
*
* All the switches are wired to one analog input, generally with resistor values 
* as shown in the circuit below. The switch pressed is determined
* from the analog value read from the port. This may vary depending on the 
* implementation of the resistor, so a translation table is implemented to allow
* the remapping of the analog value to a key number.
*
* ![LCD Shield Switches Circuit] (lcd_switch_ladder.png "Analog Ladder Circuit")
*
* The translation table must be determined separately and passed as a parameter to 
* the class constructor - the utility application Test_Analog_Keys in the examples 
* folder can be used for this purpose. The class does not copy this table to its 
* own memory, so it must remain in scope for the life of the object.
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
    uint8_t   adcTolerance; ///< Valid +/- tolerance range around adcThreshold for a key
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
  * The key definitions table is not copied by the class, so user code
  * must ensure that the table remains in scope for the life of the object 
  * created.
  *
  * \param pin    the analog pin to which the switches are connected.
  * \param kt     pointer to a table of analog value key definitions
  * \param ktSize number of elements in the kt table
  */
  MD_UISwitch_Analog(uint8_t pin, uiAnalogKeys_t* kt, uint8_t ktSize) :
    _pin(pin), _kt(kt), _ktSize(ktSize) {};

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
  * Initialise the object data. This needs to be called during setup() to initialize new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the state of the switch
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognized by a finite
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
  uint8_t   _ktSize;    ///< number of elements in analog keys table

  int16_t   _lastKeyIdx;  ///< index of the last key read
};

/**
* Extension class MD_UISwitch_Matrix.
*
* Implements keyboard matrix switches that need to be scanned to detect a keypress.
*
* ![Keypad Style Switches] (Keypad_Switches.jpg "Keypad Switches")
*
* The class will scan a key matrix that typically have circuits and connections 
* similar to that shown below. Each row and column is separately connected to a 
* digital pin, and arrays of pin numbers are passed to the class constructor 
* to allow the code to scan the matrix to determine which key has been pressed. 
* Pins should be directly connected to the matrix without pull-up or pull-down 
* resistors. The library does not make a copy of the pin arrays so they should 
* remain in scope for the life of the object.
*
* ![4x4 Matrix Keypad Circuit] (Keypad_Circuit_4x4.jpg "Keypad Circuit")
*
* The class will only return a valid key press if only one key is pressed. If 
* more than one key is pressed simultaneously, all the keys are ignored until
* just a single key is again detected.
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
  * The class will only return a valid key if just one key is pressed. If more
  * than one key is pressed simultaneously, all the keys are ignored until just
  * a single key is again detected.
  *
  * The class does not make copies of the pin array so they should 
  * remain in scope for the life of the object.
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
  * Initialize the object data. This needs to be called during setup() to initialize new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the key pressed of the switch matrix
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognized by a finite
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
};

/**
* Extension class MD_UISwitch_4017KM.
*
* Implements keyboard matrix switches implemented using 4017 decade counter
* (see https://arduinoplusplus.wordpress.com/2016/02/17/up-to-100-switches-with-3-digital-pins/).
*
* ![Key Matrix using 4017 IC] (Matrix_4017_KM.jpg "Key Matrix 4017")
*
* Using a 4017 IC, this class implements a method for reading many open/closed switches
* using only three digital I/O pins (Clock, Reset and DataIn). The class will scan a key 
* matrix that have circuits similar to that shown above. Pin numbers used are passed to 
* the class constructor allow the code to manage the 4017 IC to determine which key has 
* been pressed. Pins should be directly connected to the matrix without pull-up or 
* pull-down resistors.
*
* The class will only detect a key if there is just one key pressed. If more than one 
* key is pressed it will pause until just one key remains pressed.
*/
class MD_UISwitch_4017KM : public MD_UISwitch
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
  * The class will only return a valid key if just one key is pressed. If more
  * than one key is pressed simultaneously, all the keys are ignored until just
  * a single key is again detected.
  *
  * \param numKeys the number of keys in the 4017 matrix (rows*columns).
  * \param pinClk  pin number for the 4017 clock pin, LOW to HIGH transition.
  * \param pinRst  pin number for the 4017 reset pin (0 if not used), LOW to HIGH transition.
  * \param pinKey  pin number for the key switch output to Arduino, HIGH means key is pressed.
  */
  MD_UISwitch_4017KM(uint8_t numKeys, uint8_t pinClk, uint8_t pinKey, uint8_t pinRst) :
    _numKeys(numKeys), _pinClk(pinClk), _pinKey(pinKey), _pinRst(pinRst) {};
  
  /**
  * Class Destructor.
  *
  * Release allocated memory and does the necessary to clean up once the queue is
  * no longer required.
  */
  ~MD_UISwitch_4017KM() {};
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for core object control.
  * @{
  */
  /**
  * Initialize the object.
  *
  * Initialize the object data. This needs to be called during setup() to initialize new
  * data for the class that cannot be done during the object creation.
  */
  virtual void begin(void);

  /**
  * Return the key pressed of the switch matrix
  *
  * Return one of the keypress types depending on what has been detected.
  * The timing for each keypress starts when the first transition of the
  * switch from inactive to active state and is recognized by a finite
  * state machine whose operation is directed by the timer and option
  * values specified.
  *
  * \return one of the keyResult_t enumerated values
  */
  virtual keyResult_t read(void);
  /** @} */

protected:
  uint8_t  _numKeys; ///< total number of keys
  uint8_t  _pinClk;  ///< 4017 clock pin, LOW to HIGH transition
  uint8_t  _pinKey;  ///< key switch output to Arduino, HIGH means key is pressed	
  uint8_t  _pinRst;  ///< 4017 reset pin (0 if not used), LOW to HIGH transition

  void reset(void);  ///< reset the 4017 IC
  void clock(void);  ///< clock the 4017 IC
};

