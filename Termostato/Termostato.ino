#include <Encoder.h>

#define CLK     1
#define DATA    3
#define LATCH   2
#define ENC1    8
#define ENC2    7
#define BUTTON  0
#define LEDR    10     
#define LEDG    9
#define RELE    6
#define SEG9    4
#define SEG10   5


#define HEAT_TIMER  5000
#define BLINK_TIMER 500
#define SEG_TIMER   1800000
#define BEAT_TIMER  10000


int light[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
int oldPosition  = 0;


Encoder myEnc(ENC1, ENC2);

bool heat = false, segment  = false;
unsigned long lastPress, lastBlink, lastSegment, beat;

// TODO evaluate effect of millis resetting

void setup() {
    // put your setup code here, to run once:
    pinMode(CLK, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(LATCH, OUTPUT);
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(RELE, OUTPUT);
    pinMode(SEG9, OUTPUT);
    pinMode(SEG10, OUTPUT);

    digitalWrite(LATCH, HIGH);
    digitalWrite(CLK, LOW);
    digitalWrite(DATA, HIGH);
    digitalWrite(RELE, LOW);
    writeBars(oldPosition);
    heatSwitch(false);

    lastPress = millis();
    lastBlink = millis();
    lastSegment = millis();
    beat = millis();

    for (int i = 0; i < 5; i++) {
        digitalWrite(LEDG, HIGH);
        delay(250);
        digitalWrite(LEDG, LOW);
        delay(250);
    }
}



void loop() {
    myEnc.write(constrain(myEnc.read(), 0, 40));

    int newPosition = myEnc.read() / 4;

    if (newPosition != oldPosition) {
        oldPosition = newPosition;
        writeBars(oldPosition);
        if (oldPosition == 0) {
            heat = false;
            heatSwitch(heat);
        }
    }

    // Evaluating button press
    if (digitalRead(BUTTON) && millis() > HEAT_TIMER + lastPress && oldPosition != 0) {
        lastPress = millis();
        heat = !heat;
        heatSwitch(heat);
        // Manage other variables
        if (heat) {
            lastBlink = millis();
            lastSegment = millis();
        } else {
            writeBars(oldPosition); // Resetting blink
        }
    }

    // Blink segment
    if (millis() - BLINK_TIMER > lastBlink && heat) {
        lastBlink = millis();
        segment = !segment;
        if (segment)
            writeBars(oldPosition);
        else {
            writeBars(constrain(oldPosition - 1, 0, 10));
        }
    }

    
    // Checking if we need to decrease segment count
    if (millis () > lastSegment + SEG_TIMER && heat) {
        // Need to change state into the Encoder class
        lastSegment = millis();
        oldPosition = constrain(oldPosition - 1, 0, 10);
        myEnc.write(oldPosition * 4);
        writeBars(oldPosition);
        if (oldPosition == 0) {
            heat = false;
            heatSwitch(heat);
        }
    }

    if(millis() > BEAT_TIMER + beat) {
        beat = millis();
        digitalWrite(LEDG, HIGH);
        delay(50);
        digitalWrite(LEDG, LOW);
    }
}

void writeBars(int n) {
    // Segments managed directly
    digitalWrite(SEG10, n > 9);
    digitalWrite(SEG9, n > 8);

    // The remaing segments are just 8
    n = constrain(n, 0, 8);
    
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLK, LSBFIRST, light[n]);
    digitalWrite(LATCH, HIGH);
}

void heatSwitch(bool h) {
    digitalWrite(LEDR, h);
    digitalWrite(RELE, h);
}
