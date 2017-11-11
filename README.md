# MD_UISwitch Library

This library uniformly encapsulates the use of different types of switches 
for user input devices. The library is easily extended for additional 
switch types through a class heirarchy and inheritance model, following 
the code for existing switch types.

The library includes the following features:
- Software debounce for all switch types.
- Automatically detect switch press, double press, long press and auto repeat.
- Can work with low/high or high/low transitions.
- All software timers are configurable for fine tuning to specific applications. 
These include debounce time, double press time, long press time, and auto repeat period time.

Switch arrangements handled by the library are:
- Momentary on type switches (MD_Switch_Digital class)
- Analog resistor ladder switches (MD_Switch_Analog class)
- Keypad matrix (MD_Switch_Matrix class)
- Keypad matrix using 4017 IC (MD_Matrix_4017KM class)


[Library Documentation](https://MajicDesigns.github.io/MD_UISwitch/)