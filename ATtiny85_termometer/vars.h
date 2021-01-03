#ifndef VARS_H
#define VARS_H

const byte digit[10] =      //seven segment digits in bits
{
  B00111111, //0
  B00001100, //1
  B01100111, //2
  B01101101, //3
  B01011100, //4
  B01111001, //5
  B01111011, //6
  B00101100, //7
  B01111111, //8
  B01111101  //9
};

byte ld, rd; // left and right digit values

// to bounce between the two digits
bool flag = false; 

// LEDs
float prevMean = 0.0;
float currMean = 0.0;
int nDataPoints = 0;
unsigned long lastDataPoint;
unsigned long lastLedUpdate;
char mask = 0;
bool toggleActive = true; // Both led ON with fast switching
bool toggle = true; // Which is active now



#endif
