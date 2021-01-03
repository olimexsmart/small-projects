/*
    Frimware for digital termometer integrated
    into clock

    https://www.instagram.com/p/B5z79dPCwL8/
*/
#include "defines.h"
#include "vars.h"
#include "functions.hpp"


// #############################################

void setup() {
  pinMode(DRIVER, OUTPUT);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(CLKPIN, OUTPUT);
  pinMode(DATAPIN, OUTPUT);
  pinMode(TEMPPIN, INPUT);

  startUpSequence();

  lastLedUpdate = millis();
  lastDataPoint = millis();
}

void loop() {

  // Read temperature
  float fTemp = (((analogRead(TEMPPIN) / 1023.0) * 5.0) / GAIN);
  int temperature = round(fTemp);

  // Split in two digits
  ld = temperature / 10;
  rd = temperature % 10;

  // Update temp mean value
  if (lastDataPoint + INTERVALDATA < millis()) {
    currMean += fTemp;
    nDataPoints++;
  }

  // Update LED
  if (lastLedUpdate + INTERVALLED < millis()) {
    // Cumpute current period mean
    currMean /= nDataPoints;

    if (abs(currMean - prevMean) < NOCHANGE) {
      toggleActive = true;
    } else {
      toggleActive = false;
      mask = currMean < prevMean ? GREENLED : YELLED;
    }

    // Reset for nex update
    prevMean = currMean;
    currMean = 0.0;
    nDataPoints = 0;
    lastLedUpdate = millis();
  }

  // Manage rapid toggle to have both turned on
  if (toggleActive) {
    mask = toggle ? GREENLED : YELLED;
    toggle = !toggle;
  }

  updateDisp();

  // Doesn't need to go that fast
  delay(5);
}
