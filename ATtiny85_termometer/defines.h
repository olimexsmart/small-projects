#ifndef DEFINES_H
#define DEFINES_H

// PINS
#define DRIVER 4  // Digit selector
#define DATAPIN 0   //74HC595 Pin 14
#define LATCHPIN 1  //74HC595 Pin 12
#define CLKPIN 2   //74HC595 Pin 11
#define TEMPPIN A3   //temperature sensor pin

// CONSTANTS
#define GAIN 0.0784
#define NOCHANGE 0.25
#define INTERVALLED 60000 * 15
#define INTERVALDATA 60000 / 4

#define GREENLED 0
#define YELLED 128

#endif
