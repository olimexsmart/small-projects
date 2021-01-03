#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include "defines.h"
#include "vars.h"

// ########### FUNCTIONS ###################

//writes the temperature on display
void updateDisp() {

  digitalWrite(LATCHPIN, LOW);
  shiftOut(DATAPIN, CLKPIN, LSBFIRST, ~(digit[flag ? rd : ld] | mask));
  digitalWrite(LATCHPIN, HIGH);

  digitalWrite(DRIVER, flag);

  flag = !flag;
}

void startUpSequence() {
  rd = 8;
  ld = 8;

  mask = 255;
  for (int i = 0; i < 10; i++) {
    updateDisp();
    mask = ~mask;
    delay(200);
  }

  mask = 0;
}

#endif
