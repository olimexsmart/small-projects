/*Arduino OBD2 Bluetooth interface*/

//Version 1.5

#include <SoftwareSerial.h>
#include <DHT.h>
#include <LiquidCrystal.h>


#define TxBT A2 //Tx pin on the HC_05
#define RxBT A1 //Rx pin on the HC_05
#define EnableATBT A3 //For the luckiest EN pin, for the others PIN 34 HC_05
#define POWER 13 //Enable pin on voltage regulator
#define MAX_RETRIES 2 //Retries in communication
#define Rconst 0.244 //Constant of partition resistors
#define DIN 12 //MAX7219 pin 1
#define LOAD 11 //MAX7219 pin 12
#define CLK 10 //MAX7219 pin 13
#define MINVOLT 6.2 //Min voltage of the battery pack
#define DHTpin 2 //Data pin on DHT11
#define LUX A7 //Light resistor
#define LCDled 9 //LED on the LCD
#define SWone A5 //Switch 1
#define SWtwo A4 //Switch 2



//Class instences
SoftwareSerial SerialBT(TxBT, RxBT);
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
DHT dht;

//Variable
char buffer[41];
byte i = 0;
byte k = 0;
boolean rotation = false;
int RPM;
boolean fan = false;
byte prevT = 0;
byte T = 0;
byte dTdt = 1;
int digitBuffer[4] = {
  0
};

//unsigned long timeA = 0;
//unsigned long timeB = 0;
unsigned long timeC = 0;
//unsigned long timeD = 0;
unsigned long timeE = 0;

//Custom characters
byte bar[8] =
{
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
byte celsius[8] =
{
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
};

byte up[8] =
{
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
};

byte down[8] =
{
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};




//################## SETUP #####################
void setup()
{
  //pinMode
  pinMode(POWER, OUTPUT);
  pinMode(RxBT, OUTPUT);
  pinMode(TxBT, INPUT);
  pinMode(EnableATBT, OUTPUT);
  pinMode(DIN, OUTPUT); //data in
  pinMode(LOAD, OUTPUT); //load trigger
  pinMode(CLK, OUTPUT); //clock
  pinMode(LCDled, OUTPUT);
  pinMode(SWone, INPUT);
  pinMode(SWtwo, INPUT);

  digitalWrite(POWER, HIGH);  //Power non confirmed
  digitalWrite(LOAD, HIGH); //latch pin HIGH

  //Displays initialization
  lcd.begin(16, 2);
  //Custom characters
  lcd.createChar(0, bar);
  lcd.createChar(1, celsius);
  lcd.createChar(3, up);
  lcd.createChar(4, down);

  SetupSegments();
  luxCtrl(true);

  lcd.clear();
  lcd.print("Keep pressed...");

  //Battery level check
  if (get9Volt() < MINVOLT)
    powerOff("Low battery!");


  //Start up routine
  lcd.setCursor(0, 1);

  for (i = 1; i < 8; i++) //Loading bar
  {
    lcd.write(byte(0));
    lcd.write(byte(0));
    delay(125);
  }

  //POWER ON
  digitalWrite(POWER, LOW); //Power confirmed

  //Classes initialization
  //Serial.begin(9600); //debug
  SerialBT.begin(38400);
  dht.setup(DHTpin);

  //Setup connection
  if (!HC05_SetupCOM())
    powerOff("Failed, restart.");

}






//################## LOOP #####################
void loop()
{
  //These run as fast as possible, out of any timer
  RPM = getRPM();
  updSegements(RPM);

  //This provides auto switch off when the car is turned off
  k = 0;
  if (RPM == -1 || prevT == 255) //Two cases that may trigger
  {
    luxCtrl(true);
    lcd.clear();
    lcd.print("V1.5@olimexsmart");
    lcd.setCursor(0, 1);
    lcd.print("  @wordpress.com");

    while (RPM == -1 || prevT == 255) {
      delay(500);
      prevT = getEngineTemp();
      RPM = getRPM();
      k++;
      if (k == 15) //about a minute
        powerOff("Shutting down...");
    }
  }

  //LCD managing backlight from the switches
  luxCtrl(updLCD());

  //Check battery level
  if (get9Volt() < MINVOLT)
    powerOff("Low battery!");
}








/*
##################### FUNCTIONS ######################
 */

void HC05_ATMode() //Enables HC_05 ATmode
{
  SerialBT.flush(); //Waits for the transmission of outgoing serial data to complete
  delay(150);
  digitalWrite(EnableATBT, HIGH);
  delay(150);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void HC05_ComMode() //Enables HC_05 communicatiion mode
{
  SerialBT.flush(); //Waits for the transmission of outgoing serial data to complete
  delay(150);
  digitalWrite(EnableATBT, LOW);
  delay(150);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
boolean HC05_ATCommand(char *command) //Send an AT command
{
  boolean retry = true;
  byte retries = 0;

  while (retry && retries < MAX_RETRIES)
  {
    //Send the command
    SerialBT.print("AT");
    if (strlen(command) > 1) { //Check if it is a valid one
      SerialBT.print("+");
      SerialBT.print(command);
    }
    SerialBT.print("\r\n"); //Needed, datasheet

    timeC = millis() + 7500; //Don't wait too long
    while (SerialBT.available() <= 0 && timeC < millis());

    //Save received string in the buffer
    SerialBT.readBytes(buffer, 40);

    //Check if everything was OK
    char *ptr = strchr(buffer, 'O');
    if (ptr)
    {
      ptr++;
      if (*ptr == 'K')
        retry = false;
    }
    //If it wasn't OK, take another try
    retries++;
  }
  //If everthing was OK, return TRUE, just follows the logic
  return !retry;
}

/////////////////////////////////////////////////////
boolean HC05_COM(char *command) //Send a string to ELM327
{
  boolean retry = true;
  byte retries = 0;

  memset(buffer, '\0', 41); //Clean the buffer

  while (retry && retries < MAX_RETRIES)
  {
    SerialBT.print(command);
    SerialBT.print("\r");

    timeC = millis() + 7500; //Don't wait too long
    while (SerialBT.available() <= 0 && timeC < millis());

    //Save data in the buffer
    SerialBT.readBytes(buffer, 40);

    //Data from ELM327 is correct if there us a ">" at the end
    char *ptr = strchr(buffer, '>');
    if (ptr)
      retry = false;

    SerialBT.flush();
    retries++;
  }
  //If everthing was OK, return TRUE, just follows the logic
  return !retry;
}

/////////////////////////////////////////////////////////////////////////
boolean HC05_SetupCOM() //Estabilish connection with the ELM327
{
  //This function give a nice a loading bar and informs the user at what step the system is
  lcd.clear();
  lcd.print("BT start    1/13");
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); //Write a bar token

  //Bluetooth connection setup
  HC05_ATMode();

  //It never hurts
  if (!HC05_ATCommand("RESET")) return false;
  lcd.setCursor(0, 0);
  lcd.print("BT start    2/13");
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); //write a bar token
  delay(900);

  //Master role
  if (!HC05_ATCommand("ROLE=1")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Role = 1    3/13");
  lcd.setCursor(1, 1);
  lcd.write(byte(0)); //write a bar token
  delay(250);


  if (!HC05_ATCommand("CMODE=0")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Cmode = 0   4/13");
  delay(250);


  if (!HC05_ATCommand("BIND=19, 5D, 26A42A")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Bind addr   5/13");
  lcd.setCursor(2, 1);
  lcd.write(byte(0)); //write a bar token
  delay(250);


  if (!HC05_ATCommand("INIT")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Init        6/13");
  lcd.setCursor(3, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(250);


  if (!HC05_ATCommand("PAIR=19, 5D, 26A42A,20")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Pair addr   7/13");
  lcd.setCursor(6, 1);
  lcd.write(byte(0)); //write a bar token
  delay(250);


  if (!HC05_ATCommand("LINK=19, 5D, 26A42A")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Link addr   8/13");
  lcd.setCursor(7, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(300);


  /*
  If everything was fine, it's time to start communication with ELM327
   */

  //OBD2 interface setup
  lcd.setCursor(0, 0);
  lcd.print("OBD2 start  9/13");
  lcd.setCursor(9, 1);
  lcd.write(byte(0)); //write a bar token
  HC05_ComMode();

  //Reset never hurts
  if (!HC05_COM("ATZ")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Reset ELM  10/13");
  lcd.setCursor(10, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(400);

  //Telemetry mode select, mode zero
  if (!HC05_COM("ATSP0")) return false;
  lcd.setCursor(0, 0);
  lcd.print("Mode zero  11/13");
  lcd.setCursor(12, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(400);

  //Ask for PIDs
  if (!HC05_COM("0100")) return false;
  lcd.setCursor(0, 0);
  lcd.print("PIDs...    12/13");
  lcd.setCursor(14, 1);
  lcd.write(byte(0)); //write a bar token
  delay(850);

  //Print last token bar and leave
  lcd.setCursor(0, 0);
  lcd.print("Connected  13/13");
  lcd.setCursor(15, 1);
  lcd.write(byte(0)); //write a bar token

  return true;
}

/*
All available PIDs are on Wikipedia: http://en.wikipedia.org/wiki/OBD-II_PIDs
 The functions that follows retreive the information needed sending strings,
 called PIDs (Parameter IDs).
 Every response has a specific layout, but in general the packet is similar to this:
 (assuming that the query PID is 01xx1) 01xx141 xx AA BB >
 All values are expressed in HEX numbers in specific:
 xx  PID specific, not always is repeted after the 41
 AA  first half of data
 BB  second half of data, optional
 >   needed for integrity control
 */
/////////////////////////////////////////////////////////////////
int getEngineTemp() //Get engine temperature
{

  //Retreive the string
  if (!HC05_COM("01051"))
    return -1;

  //These characters ensure valid data
  if (buffer[6] == '4' && buffer[7] == '1')
  {
    for (i = 12; i < 14; i++)
    {
      //HEX to DEC conversion
      if (buffer[i] >= 'A' && buffer[i] <= 'F')
        buffer[i] -= 55;
      if (buffer[i] >= '0' && buffer[i] <= '9')
        buffer[i] -= 48;
    }
    //HEX to DEC conversion
    byte B = (buffer[12] * 16) + buffer[13];

    return B - 40; //OBD2 temperature formula
  }
  else
    return -1;
}


/////////////////////////////////////////////////////////////
int getRPM()
{
  //Retreive the string
  if (!HC05_COM("010C1"))
    return -1;

  //These characters ensure valid data
  if (buffer[6] == '4' && buffer[7] == '1' && buffer[9] == '0' && buffer[10] == 'C')
  {
    for (i = 12; i < 17; i++)
    {
      if (i == 14) i++; //There is a space between the two numbers
      //HEX to DEC conversion
      if (buffer[i] >= 'A' && buffer[i] <= 'F')
        buffer[i] -= 55;
      if (buffer[i] >= '0' && buffer[i] <= '9')
        buffer[i] -= 48;
    }
    //HEX to DEC conversion
    byte A = (buffer[12] * 16) + buffer[13];
    byte B = (buffer[15] * 16) + buffer[16];

    return ((A * 256) + B) / 4; //OBD2 RPM formula
  }
  else
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////
float get12Volt() //Get car battery voltage, it's an ELM327 internal AT command
{
  //Retreive the string
  if (!HC05_COM("ATRV"))
    return -1;

  for (i = 5; i < 9; i++) //To DEC conversion
  {
    if (buffer[i] >= '0' && buffer[i] <= '9')
      buffer[i] -= 48;
  }
  //Build the number according to datasheet
  float t = (float) buffer[8] / 10;
  return (buffer[5] * 10) + buffer[6] + t;
}

////////////////////////////////////////////////////////////////////////////////
float get9Volt() //Get voltage of the battery pack from the voltage partitor
{
  float v = analogRead(A0) * 5 / 1023.0;
  return (v / Rconst);
}

//////////////////////////////////////////////////////////////////////////////////////
void SetupSegments() //MAX7219 needs a startup sequence to set up the way you want to use it
{
  //Deactivating shutdown
  serOut(B00001100, B11111111);
  //Deactivating test mode
  serOut(B00001111, B11110110);
  //Set to decode mode B
  serOut(B00001001, B11111111);
  //Set intensity to 8, half value
  serOut(B00001010, B11111000);
  //Set scan limit to 4 digits
  serOut(B00001011, B11111011);

  //Show that all is good lighting everything up
  for (i = 1; i < 5; i++)
    serOut(i, B10001000);
}

///////////////////////////////////////////////////////////////////////////////
void updSegements(int n) //Takes a number and displays it on the screen
{
  if (n < 0) //Invalid RPM value
  { //Display HELP, the only word the chip knows
    serOut(1, 14);
    serOut(2, 13);
    serOut(3, 11);
    serOut(4, 12);
  }
  else if (n < 10000) { //Four digit, max is 9999
    //We need to divide the numbers in digits
    digitBuffer[0] = n % 10; //Units

    digitBuffer[1] = n % 100; //Tens
    digitBuffer[1] /= 10;

    digitBuffer[2] = n % 1000; //Hundreds
    digitBuffer[2] /= 100;

    digitBuffer[3] = n % 10000; //Thousands
    digitBuffer[3] /= 1000;

    for (i = 1; i < 5; i++) //Push digits in the chip
    {
      //If the number is shorter than four digits, switch off the unused digits
      if (i == 2 && n < 10) digitBuffer[1] = 15;
      if (i == 3 && n < 100) digitBuffer[2] = 15;
      if (i == 4 && n < 1000) digitBuffer[3] = 15;

      serOut(i, (byte)digitBuffer[i - 1]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////
void serOut(byte op, byte var) //Takes the OP code and the data and latches it in
{
  digitalWrite(LOAD, LOW);
  shiftOut(DIN, CLK, MSBFIRST, op);
  shiftOut(DIN, CLK, MSBFIRST, var);
  digitalWrite(LOAD, HIGH);
}

/////////////////////////////////////////////////////
boolean updLCD()
{
  rotation = !rotation;
  /*
   * Temperature of the engine is managed inside this fx,
   * it needs a lot of code and it manages also
   * emergency situation disregard of the rotation we are
   * currently in. The rotation status is updated on the
   * next call of updLCD()
   */
  tempStatus();


  //Read the switches
  if (digitalRead(SWtwo) && digitalRead(SWone)) // RPM only
  {
    lcd.clear();
    return false; //lcd off
  }

  if (digitalRead(SWtwo)) //Stops to the current rotation if activated
    rotation = false;

  if (digitalRead(SWone))  //Temperature only
    rotation = true;


  if (rotation)
  {
    float vc = get12Volt();
    float vi = get9Volt();
    lcd.clear();

    lcd.print("VC:");
    lcd.print(vc, 1);
    lcd.print("V  T:");

    lcd.print((int)dht.getTemperature());
    lcd.write(byte(1));
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("VI:");
    lcd.print(vi, 1);
    lcd.print("V  H:");

    lcd.print((int)dht.getHumidity());
    lcd.print("%");
  }
  else
  {
    lcd.clear();
    if (fan)
      lcd.print("Ventola");
    else if (T > 70)
      lcd.print("Normale");
    else if (T <= 70)
      lcd.print("Freddo");

    //print current T
    lcd.setCursor(12, 0);
    lcd.print(T);
    lcd.write(byte(1));
    lcd.print("C");

    //rising or falling
    if (timeE < millis() - 20000) //Don't update too fast
    {
      if (T > prevT)
        dTdt = 2;
      else if (T == prevT)
        dTdt = 1;
      else
        dTdt = 0;

      timeE = millis();
      prevT = T;
    }

    //Display a row of symbols showing the evolution of T
    lcd.setCursor(0, 1);
    if (dTdt == 2)
      for (i = 0; i <= 16; i++)
        lcd.write(byte(3));
    else if (dTdt == 1)
      for (i = 0; i <= 16; i++)
        lcd.print("=");
    else if (dTdt == 0)
      for (i = 0; i <= 16; i++)
        lcd.write(byte(4));
  }

  return true; //lcd on
}

///////////////////////////////////////////////////
void powerOff(char *message)
{
  //Display HELP, the only word the MAX7219 knows
  serOut(1, 14);
  serOut(2, 13);
  serOut(3, 11);
  serOut(4, 12);

  lcd.clear();
  luxCtrl(true);
  lcd.print(message);
  //leave some time to read
  delay(10000);
  //POWER OFF
  digitalWrite(POWER, HIGH);

  while (true);
}

///////////////////////////////////////////////////
//Adjust backlight intensity and switch on\off the LCD
void luxCtrl(boolean lcd)
{
  int lux = analogRead(LUX);

  serOut(10, map(lux, 0, 1023, 10, 15));

  if (lcd) {
    lux = map(lux, 0, 1023, 128, 255);
    analogWrite(LCDled, lux);
  }
  else
    analogWrite(LCDled, 0);
}


/////////////////////////////////////////////////////////
//Temp monitoring function
void tempStatus()
{
  T = getEngineTemp();

  //The fan activates at 98Â° and deactivates at 93Â°
  if (!fan && T >= 98)
    fan = true;
  if (fan && T < 94)
    fan = false;

  if (T == 255)
  {
    luxCtrl(true);
    lcd.clear();
    lcd.print("V1.4@olimexsmart");
    lcd.setCursor(0, 1);
    lcd.print("  @wordpress.com");

    delay(30000);

    powerOff("Shutting down...");
  }
  
  //EMERGENCY LOOP
  if (T > 99)
  {
    boolean onOff = true;

    //Display HELP, the only word the MAX7219 knows
    serOut(1, 14);
    serOut(2, 13);
    serOut(3, 11);
    serOut(4, 12);

    lcd.clear();
    lcd.print("!! EMERGENCY !!");
    lcd.setCursor(6, 1);
    lcd.print(T);
    lcd.setCursor(10, 1);
    lcd.write(byte(1));
    lcd.print("C");

    while (T > 99)
    {
      T = getEngineTemp();
      lcd.setCursor(6, 1);
      lcd.print(T);

      if (onOff) //Alternate shutdown mode
      {
        serOut(B00001100, B11111110);
        onOff = false;
      } else {
        serOut(B00001100, B11111111);
        onOff = true;
      }
    }
  } //EMERGENCY LOOP CLOSED
}

