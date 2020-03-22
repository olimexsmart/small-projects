/*
  Pc fan controller for the DIY iMac of poor people.
  The poit is to read out the voltage of the only
  system fan, 0-5V and apply it to an additional fan
  than ranges from 0 to 12V. The controller knows when
  the PC is off and on through a sense wire from the usb 5V.
  On board there are two trimmers, one that directly controls
  the fan speed, basically a offset. The other one adjusts
  the gain of the voltage mapping.

  KEEP IN MIND -> The PWM logic is negated
 */
//#include <UserTimer.h>

#define FAN 1
#define USB 0
#define GAIN A1
#define OFFSET A3
#define SENSE A2

bool startUp = false;
unsigned int offset = 0;
unsigned int gain = 0;
unsigned int sense = 0;
unsigned int output = 0;


void setup()
{
  TCCR0B = TCCR0B & 0b11111000 | 0x01; //Sets the PWM frequency to about 16kHz

  pinMode(FAN, OUTPUT);
  pinMode(USB, INPUT);
  pinMode(GAIN, INPUT);
  pinMode(OFFSET, INPUT);
  pinMode(SENSE, INPUT);

  //StartUpTest();
  ApplyToFAN(0);
}


void loop()
{
  if (digitalRead(USB) && !startUp) //From off to on
  {
    startUp = true;
    StartUpTest();
  }

  if (!digitalRead(USB) && startUp) //From on to off
  {
    startUp = false;
    ApplyToFAN(0);
  }

  if(startUp)
  {
    offset = analogRead(OFFSET);
    gain = analogRead(GAIN);
    sense = analogRead(SENSE);

    gain = map(gain, 0, 1023, 1, 5); //Map the gain from 0 to 10
    output = sense * gain + offset; //Open loop proportional controller
    output = constrain(output, 0, 1023);
    output = map(output, 0, 1023, 0, 255);

    ApplyToFAN(output);
  }

}




void StartUpTest()
{
  for (byte i = 0; i < 255; i++)
  {
    ApplyToFAN(i);
    delay(15);
  }

  for (byte i = 255; i > 0; i--)
  {
    ApplyToFAN(i);
    delay(15);
  }
}

void ApplyToFAN(int value)
{
  analogWrite(FAN, 255 - value);
}











