/*Arduino OBD2 Bluetooth interface*/

//Version 1.3

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
byte rotation = 0;
int RPM;
boolean LCD = true;
boolean fan = false;
byte prevT = 0;
byte dTdt = 1;
int digitBuffer[4] = {
  0
};

unsigned long timeA = 0;
unsigned long timeB = 0;
unsigned long timeC = 0;
unsigned long timeD = 0;
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
  lcd.createChar(0, bar);
  lcd.createChar(1, celsius);
  lcd.createChar(3, up);
  lcd.createChar(4, down);

  lcd.begin(16, 2);
  analogWrite(9, 200); //Arbitrary value
  //Battery level check
  if (get9Volt() < MINVOLT)
    powerOff("LOW BATTERY");

  //Start up routine
  lcd.print("KEEP PRESSED");
  lcd.setCursor(0, 1);
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);

  for (i = 1; i < 8; i++)
  {
    lcd.write(byte(0));
    lcd.write(byte(0));
    delay(100);
  }

  //POWER ON
  digitalWrite(POWER, LOW);

  //pinMode
  pinMode(RxBT, OUTPUT);
  pinMode(TxBT, INPUT);
  pinMode(EnableATBT, OUTPUT);
  pinMode(DIN, OUTPUT); //data in
  pinMode(LOAD, OUTPUT); //load trigger
  pinMode(CLK, OUTPUT); //clock
  pinMode(LCDled, OUTPUT);
  pinMode(SWone, INPUT);
  pinMode(SWtwo, INPUT);

  digitalWrite(LOAD, HIGH); //latch pin HIGH

  //Classes initialization
  Serial.begin(9600); //debug
  SerialBT.begin(38400);
  SetupSegments();
  dht.setup(DHTpin);

  //Setup connection
  if (HC05_SetupCOM()) {
    lcd.setCursor(0, 0);
    lcd.print("CONNECTED   100%");
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); //write a bar token
  }
  else
    powerOff("FAILED");

  //Initialize timers
  timeA = millis();
  timeB = millis();
  timeD = millis();
}






//################## LOOP #####################
void loop()
{
  //These run as fast as possible, out of any timer
  RPM = getRPM();
  updSegements(RPM);

  //This provides auto switch off when the car is turned off
  k = 0;
  if (RPM == -1 || prevT == 255)
  {
    luxCtrl(true);
    lcd.clear();
    lcd.print("V1.3@olimexsmart");
    lcd.setCursor(0, 1);
    lcd.print("  @wordpress.com");

    while (RPM == -1 || prevT == 255) {
      delay(500);
      prevT = getEngineTemp();
      RPM = getRPM();
      k++;
      if (k == 10) //about a minute
        powerOff("SHUTTING DOWN");
    }
  }

  //LCD UPDATE
  LCD = updLCD();
  timeA = millis();
  luxCtrl(LCD);


  //The battery level can't change instantly, every minute is quite enough
  if (timeB < millis() - 60000) //Checking battery level
  {
    if (get9Volt() < MINVOLT)
      powerOff("LOW BATTERY");
    timeB = millis();
  }
}








/*
##################### FUNCTIONS ######################
 */

void HC05_ATMode() //Enables HC_05 ATmode
{
  SerialBT.flush();
  delay(150);
  digitalWrite(EnableATBT, HIGH);
  delay(150);
}

////////////////////////////////////////////////////////
void HC05_ComMode() //Enables HC_05 communicatiion mode
{
  SerialBT.flush();
  delay(150);
  digitalWrite(EnableATBT, LOW);
  delay(150);
}

/////////////////////////////////////////////////////////
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
  lcd.print("CONNECTING    0%");
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); //Write a bar token

  //Bluetooth connection setup
  HC05_ATMode();

  //It never hurts
  if (!HC05_ATCommand("RESET")) return false;
  lcd.setCursor(0, 0);
  lcd.print("BT START      3%");
  lcd.setCursor(0, 1);
  lcd.write(byte(0)); //write a bar token
  delay(800);

  //Master role
  if (!HC05_ATCommand("ROLE=1")) return false;
  lcd.setCursor(0, 0);
  lcd.print("ROLE         11%");
  lcd.setCursor(1, 1);
  lcd.write(byte(0)); //write a bar token
  delay(150);


  if (!HC05_ATCommand("CMODE=0")) return false;
  lcd.setCursor(0, 0);
  lcd.print("CMODE        19%");
  delay(150);


  if (!HC05_ATCommand("BIND=19, 5D, 26A42A")) return false;
  lcd.setCursor(0, 0);
  lcd.print("BIND         23%");
  lcd.setCursor(2, 1);
  lcd.write(byte(0)); //write a bar token
  delay(150);


  if (!HC05_ATCommand("INIT")) return false;
  lcd.setCursor(0, 0);
  lcd.print("INIT         41%");
  lcd.setCursor(3, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(150);


  if (!HC05_ATCommand("PAIR=19, 5D, 26A42A,20")) return false;
  lcd.setCursor(0, 0);
  lcd.print("PAIR         53%");
  lcd.setCursor(6, 1);
  lcd.write(byte(0)); //write a bar token
  delay(150);


  if (!HC05_ATCommand("LINK=19, 5D, 26A42A")) return false;
  lcd.setCursor(0, 0);
  lcd.print("LINK         69%");
  lcd.setCursor(7, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(200);


  /*
  If everything was fine, it's time to start communication with ELM327
   */

  //OBD2 interface setup
  lcd.setCursor(0, 0);
  lcd.print("OBD START    72%");
  lcd.setCursor(9, 1);
  lcd.write(byte(0)); //write a bar token
  HC05_ComMode();

  //Reset never hurts
  if (!HC05_COM("ATZ")) return false;
  lcd.setCursor(0, 0);
  lcd.print("RESET        84%");
  lcd.setCursor(10, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(300);

  //Telemetry mode select, mode zero
  if (!HC05_COM("ATSP0")) return false;
  lcd.setCursor(0, 0);
  lcd.print("MODE ZERO    93%");
  lcd.setCursor(12, 1);
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  lcd.write(byte(0)); //write a bar token
  delay(300);

  //Ask for PIDs
  if (!HC05_COM("0100")) return false;
  lcd.setCursor(0, 0);
  lcd.print("PIDS...      97%");
  lcd.setCursor(14, 1);
  lcd.write(byte(0)); //write a bar token
  delay(750);

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

/////////////////////////////////////////////////////////////////////////
byte getLoad()
{
  //Retreive the string
  if (!HC05_COM("01041"))
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

    return (B * 100) / 255; //OBD2 load formula
  }
  else
    return -1;
}

/////////////////////////////////////////////////////////////////
byte getThrottle()
{
  //Retreive the string
  if (!HC05_COM("01111"))
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

    return (B * 100) / 255; //OBD2 throttle formula
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
  //Build the number
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
  byte th;
  byte ld;

  if (digitalRead(SWtwo) && digitalRead(SWone))
  {
    lcd.clear();
    return false;
  }

  if (!digitalRead(SWtwo))
  {
    rotation++;
    if (rotation == 4)
      rotation = 0;
  }

  if (digitalRead(SWone))
    rotation = 0;

  //qui funzione monitoraggio temperatura
  //dal case viene tolto il caso zero e lcd aggiornato da dentro la funzione
  tempStatus();


  switch (rotation)
  {
    case 1: //all engine info
      th = getThrottle();
      ld = getLoad();
      lcd.clear();
      lcd.print("ENGINE");
      lcd.setCursor(10, 0);
      lcd.print("T:");
      lcd.print(prevT);
      lcd.write(byte(1));
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("L:");
      lcd.print(ld);
      lcd.print("%");
      lcd.setCursor(9, 1);
      lcd.print("TH:");
      lcd.print(th);
      lcd.print("%");
      break;

    case 2: //voltages info
      lcd.clear();
      lcd.print("VOLTAGE");
      lcd.setCursor(11, 0);
      lcd.print(get9Volt());
      lcd.print("V");
      lcd.setCursor(1, 1);
      lcd.print("CAR:");
      lcd.print(get12Volt());
      lcd.print("V");
      break;

    case 3: //inside info
      lcd.clear();
      lcd.print("INSIDE");
      lcd.setCursor(0, 1);
      lcd.print("HUM:");
      lcd.print((int)dht.getHumidity());
      lcd.print("%");
      lcd.setCursor(10, 1);
      lcd.print("T:");
      lcd.print((int)dht.getTemperature());
      lcd.write(byte(1));
      lcd.print("C");

      break;
  }

  return true;

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
  lcd.print(message);
  //let some time to read
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
  byte T = getEngineTemp();

  //The activates at 98Â° and deactivates at 93Â°
  if (!fan && T >= 98)
    fan = true;
  if (fan && T < 94)
    fan = false;

  //EMERGENCY LOOP
  if (T > 99)
  {
    boolean onOff = true;
    //Max brightness
    serOut(10, 15);
    analogWrite(LCDled, 255);

    //Display HELP, the only word the MAX7219 knows
    serOut(1, 14);
    serOut(2, 13);
    serOut(3, 11);
    serOut(4, 12);

    lcd.clear();
    lcd.print("!! ATTENZIONE !!");
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

      if (onOff)
      {
        serOut(B00001100, B11111110);
        onOff = false;
      } else {
        serOut(B00001100, B11111111);
        onOff = true;
      }
    }
  } //EMERGENCY LOOP CLOSED

  if (rotation != 0)
  {
    prevT = T;
    return;
  }

  //If rotation IS at zero, update LCD
  lcd.clear();
  if (fan)
    lcd.print("VENTOLA");
  else if (T > 70)
    lcd.print("NORMALE");
  else if (T <= 70)
    lcd.print("A FREDDO");

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
    else if (T = prevT)
      dTdt = 1;
    else
      dTdt = 0;

  }
  
  lcd.setCursor(0, 1);
  if (dTdt == 2)
    for (i = 0; i <= 16; i++)
      lcd.write(byte(3));
  else if (dTdt == 1)
    for (i = 0; i <= 16; i++)
      lcd.print("=");
  else
  {
    for (i = 0; i <= 16; i++)
      lcd.write(byte(4));
  }
  timeE = millis();


  prevT = T;
}








