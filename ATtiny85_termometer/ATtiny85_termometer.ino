/*
    created by Rui Santos, http://randomnerdtutorials.com
    Temperature Sensor Displayed on 4 Digit 7 segment common anode
    2013
*/
#define DRIVER 4

#define DATAPIN 0   //74HC595 Pin 14
#define LATCHPIN 1  //74HC595 Pin 12
#define CLKPIN 2   //74HC595 Pin 11

#define TEMPPIN A3   //temperature sensor pin

#define GAIN 0.0784

#define INTERVAL 60000

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

bool flag = false; // to bounce between the two

float prevTemp = 0.0;
unsigned long lastTime;
char mask = 0;

void setup() {
    pinMode(DRIVER, OUTPUT);
    pinMode(LATCHPIN, OUTPUT);
    pinMode(CLKPIN, OUTPUT);
    pinMode(DATAPIN, OUTPUT);
    pinMode(TEMPPIN, INPUT);

    lastTime = millis();
}



//writes the temperature on display
void updateDisp() {

    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLKPIN, LSBFIRST, ~(digit[flag ? rd : ld] | mask));
    digitalWrite(LATCHPIN, HIGH);

    digitalWrite(DRIVER, flag);

    flag = !flag;
}

void loop() {

    float fTemp = (((analogRead(TEMPPIN) / 1023.0) * 5.0) / GAIN);

    //float fTemp = (((analogRead(TEMPPIN) / 1024.0) * 5000.0) / 10.0 );

    int temperature = (int) fTemp;

    ld = temperature / 10;
    rd = temperature % 10;

    if (lastTime + INTERVAL < millis()) {
        mask = fTemp > prevTemp ? 0 : 128;

        prevTemp = fTemp;
        lastTime = millis();
    }

    updateDisp();
    delay(1);
}
