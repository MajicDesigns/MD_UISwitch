## MD_UISwitch Universal User Interface Switch Library

This library uniformly encapsulates the use of different types of switches 
for user input devices. The library is easily extended for additional 
switch types through a class hierarchy and inheritance model, following 
the code for existing switch types.

The library includes the following features:
- Software debounce for all switch types.
- Automatically detect switch press, double press, long press and auto repeat.
- Can work with low/high or high/low transitions.
- Timers are all software configurable - debounce time, double press time, long press time, and auto repeat period time.

Switch arrangements handled by the library are:
- Momentary on type switches (MD_Switch_Digital class)
- Analog resistor ladder switches (MD_Switch_Analog class)
- Keypad matrix (MD_Switch_Matrix class)
- Keypad matrix using 4017 IC (MD_Matrix_4017KM class)

If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)

[Library Documentation](https://MajicDesigns.github.io/MD_UISwitch/)