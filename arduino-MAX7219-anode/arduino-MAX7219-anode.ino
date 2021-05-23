/*
  Code by  OlimexSmart - Luca Olivieri
  http://olimexsmart.wordpress.com/
  Please keep this head comment :)

  Sketch for testing the MAX7219 with a common anode display.
  The wiring is simple

  MAX7219           7 - DISPLAY
  D0 to D7     ---> A to DP
  SEG G to DP  ---> 1 to 8 digit, first from left

  Don't forgot Iset on the MAX7219!
*/

#define interval 100
#define DIN 4
#define LOAD 3
#define CLK 2
#define DIGITS 4

int digit[DIGITS] = {
  0
};
const byte segments[11] = {
  B11111100,
  B01100000,
  B11011010,
  B11110010,
  B01100110,
  B10110110,
  B10111110,
  B11100000,
  B11111110,
  B11110110,
  B00000000
};


int count = 0;
boolean flag = true;
unsigned long timea = 0;
unsigned long timeb = 0;


void setup()
{
  pinMode(DIN, OUTPUT); //data in
  pinMode(LOAD, OUTPUT); //load trigger
  pinMode(CLK, OUTPUT); //clock

  digitalWrite(LOAD, HIGH); //latch pin HIGH

  serOut(0x0F, 0x00);  //deactivating test mode
  serOut(0x09, 0x00);  //setting to no decode
  serOut(0x0A, 0x07);  //setting intensity
  serOut(0x0B, 0x07);  //setting scan limit to 8 digits
  serOut(0x0C, 0x01);   //deactivating shutdown

}

void loop()
{
  timeb = millis();
  if (timeb - timea > interval) {
    if (flag)
      count++;
    else
      count--;

    if (count == 9999)
      flag = false;

    if (count == 0)
      flag = true;

    timea = timeb;

    dispInt(count);
  }
}

void serOut(byte op, byte var)
{
  digitalWrite(LOAD, LOW);
  shiftOut(DIN, CLK, MSBFIRST, op);
  shiftOut(DIN, CLK, MSBFIRST, var);
  digitalWrite(LOAD, HIGH);
}

void dispInt(int n)
{
  //dividing the number to display in digits
  for (byte m = 0; m < DIGITS; m++)
  {
    digit[m] = n % (int)pow(10, m + 1);
    digit[m] /= pow(10, m);

    //this line erases useless zero of unused digits
    if (digit[m] == 0 && n < pow(10, m) && n > 0) digit[m] = 10;
  }

  byte temp = 0;
  byte packet;
  int s = 0;

  for (byte i = 0; i < 8; i++) //packet count
  {
    packet = 0;

    for (byte k = 0; k < DIGITS ; k++) //packet fill
    {
      temp = segments[digit[k]] & B10000000 >> i; // masking unused bits
      s = 7 - k - i;  //shifting bit at the right position
      if (s >= 0) //negative shifting is undefined in C
        temp >>= s;
      else
        temp <<= -s;
      packet = temp | packet; //filling them in the packet
    }

    serOut(i + 1, packet); //First opcode is for no-op, digits counting from one
  }
}
