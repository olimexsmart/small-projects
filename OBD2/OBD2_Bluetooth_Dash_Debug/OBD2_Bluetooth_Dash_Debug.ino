/*Arduino OBD2 Bluetooth interface*/

#include <SoftwareSerial.h>
//#include <DHT.h>
#include <LiquidCrystal.h>


#define TxBT 11
#define RxBT 12
#define EnableATBT 13
#define MAX_RETRIES 3


SoftwareSerial SerialBT(TxBT, RxBT);
LiquidCrystal lcd(6, 7, 2, 3, 4, 5);
//DHT dht;
char buffer[41];
unsigned long time;
byte i = 0;

//debugging
char command[41];
boolean mode = false;
boolean notNow = false;
int rpm;

void setup()
{
  lcd.print("Booting...");

  pinMode(RxBT, OUTPUT);
  pinMode(TxBT, INPUT);
  pinMode(EnableATBT, OUTPUT);

  digitalWrite(EnableATBT, LOW);    //Set HC-05 to Com mode


  Serial.begin(9600);
  SerialBT.begin(38400);
  lcd.begin(16, 2);
  digitalWrite(9, HIGH);
  //  dht.setup(2); // data pin 2

    //Setup HC-05
  lcd.print("Entering in ATMODE");
  HC05_ATMode();

  lcd.clear();   
  lcd.print("Setting up connection...");
  if(HC05_SetupCOM()){
    lcd.clear();     
    lcd.print("Connected to the car.");
  }
  else
  {
    lcd.clear();     
    lcd.print("Connection failed.");
    while(true);
  }

  time = millis();
}

//################## LOOP #####################
void loop()
{
  while(Serial.available() <= 0)
  {
    if((time < millis() - 500) && SerialBT.available() > 0){ //check sometimes if something was received
      while(SerialBT.available() > 0)
        Serial.write(SerialBT.read());
      time = millis();
    }
  }

  Serial.println();

  Serial.readBytes(command, 40);

  for(byte i = 0; i < 40; i++)
    command[i] = toupper(command[i]);

  Serial.println(command);

  //This segment switches between the operating modes
  if(strcasecmp(command, "COM") == 0)
  {
    HC05_ComMode();
    Serial.println("HC-05 entered in communication mode.");
    mode = false;
    notNow = true;
  }

  if(strcasecmp(command, "ATMODE") == 0)
  {
    HC05_ATMode();
    Serial.println("HC-05 entered in AT mode.");
    mode = true;
    notNow = true;
  }

  //Test of the various functions
  if(strcasecmp(command, "RPM") == 0)
  {
    rpm = getRPM();
    Serial.print("Test RPM request. Result: "); 
    Serial.println(rpm);
  
    notNow = true;
  }

  if(strcasecmp(command, "RPMmax") == 0)
  {
    time = millis();
    i = 0;
    while(time > millis() - 10000){
      rpm = getRPM();
      i++;
    }
    Serial.print("Test RPM request. Max repetitions in 10 sec: "); 
    Serial.println(i);
  
    notNow = true;
  }
  if(strcasecmp(command, "DATA") == 0)
  {
    int t;
    t = getLoad();
    Serial.print("Engine load: "); 
    Serial.println(t);
    t = getThrottle();
    Serial.print("Engine throttle: ");
    Serial.println(t);
    t = getEngineTemp();
    Serial.print("Engine temperature: "); 
    Serial.println(t);
    float v = get12Volt();
    Serial.print("Engine battery voltage: ");
    Serial.println(v);

    notNow = true;
  }


  if(!notNow){    //don't want to send switching-mode strings
    if(mode)
      HC05_ATCommand(command);
    else
      HC05_COM(command);
  }

  notNow = false;
  Serial.println(buffer);
  Serial.println("STOP");

  memset(command, '\0', 41);
}



/*
##################### FUNCTIONS ######################
 */
void HC05_ATMode()
{
  SerialBT.flush();
  delay(250);
  digitalWrite(EnableATBT, HIGH);
  delay(250);
}
////////////////////////////////////////////////////////
void HC05_ComMode()
{
  SerialBT.flush();
  delay(250);
  digitalWrite(EnableATBT, LOW);
  delay(250);
}

/////////////////////////////////////////////////////////
boolean HC05_ATCommand(char *command)
{
  boolean retry = true;
  byte retries = 0;

  while(retry && retries < MAX_RETRIES)
  {
    //Send the command
    SerialBT.print("AT");
    if(strlen(command) > 1){ //Check if it is a valid one
      SerialBT.print("+");
      SerialBT.print(command);
    }
    SerialBT.print("\r\n");

    while(SerialBT.available() <= 0);              //wait while no data

    SerialBT.readBytes(buffer, 40);

    char *ptr = strchr(buffer, 'O');
    if(ptr)
    {
      ptr++;
      if(*ptr == 'K')
        retry = false;
      else
        retries++;
    }
  }
  return !retry;
}

/////////////////////////////////////////////////////
boolean HC05_COM(char *command)
{
  boolean retry = true;
  byte retries = 0;
  char *c;

  memset(buffer, '\0', 41); //Clean the buffer

  while(retry && retries < 5)
  {
    SerialBT.print(command);
    SerialBT.print("\r");

    while(SerialBT.available() <= 0);

    SerialBT.readBytes(buffer, 40);

    c = strchr(buffer, '>');
    if(c)
      retry = false;
    else
      retries++;

    SerialBT.flush();
  }

  return !retry;
}

////////////////////////////////////////////////////
boolean HC05_SetupCOM()
{
  i = 0;
  lcd.clear();
  lcd.println("In setup DB");
  //Bluetooth connection setup
  if(HC05_ATCommand("RESET")) i++;
  delay(2000);
  lcd.clear();    
  lcd.println("role DB");
  if(HC05_ATCommand("ROLE=1")) i++;
  delay(500);
  lcd.clear();      
  lcd.println("cmode DB");
  if(HC05_ATCommand("CMODE=0")) i++;
  delay(500);
  lcd.clear();      
  lcd.println("bind DB");
  if(HC05_ATCommand("BIND=19, 5D, 26A42A")) i++;
  delay(500);
  lcd.clear();   
  lcd.print("Initialization...");
  if(HC05_ATCommand("INIT")) i++;
  delay(750);
  if(HC05_ATCommand("PAIR=19, 5D, 26A42A,20")) i++;
  delay(500);
  lcd.clear();   
  lcd.print("Linking...");
  if(HC05_ATCommand("LINK=19, 5D, 26A42A")) i++;
  delay(1000);

  if(i != 7)
    return false;

  //If everything was fine, it's time to start communication
  lcd.clear();  
  lcd.print("Entering in COM mode...");
  HC05_ComMode();
  //OBD2 interface setup
  i = 0;
  lcd.clear();   
  lcd.print("OBD2 interfacing...");
  if(HC05_COM("ATZ")) i++;
  delay(1500);
  if(HC05_COM("ATSP0")) i++;  
  delay(1500);
  if(HC05_COM("0100")) i++;
  delay(3000);

  if(i != 3)
    return false;

  return true;
}

/////////////////////////////////////////////////////////////////
int getEngineTemp()
{
  //memset(command, '\0', 41);

  if(!HC05_COM("01051"))
    return -1;


  if(buffer[6] == '4' && buffer[7] == '1')
  {
    for(i = 12; i < 14; i++)
    {
      if(i == 11) i++; //There is a space between the two numbers

      if(buffer[i] >= 'A' && buffer[i] <= 'F')
        buffer[i] -= 55;
      if(buffer[i] >= '0' && buffer[i] <= '9')
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
  //memset(command, '\0', 41);

  if(!HC05_COM("01041"))
    return -1;


  if(buffer[6] == '4' && buffer[7] == '1')
  {
    for(i = 12; i < 14; i++)
    {
      if(i == 11) i++; //There is a space between the two numbers

      if(buffer[i] >= 'A' && buffer[i] <= 'F')
        buffer[i] -= 55;
      if(buffer[i] >= '0' && buffer[i] <= '9')
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
  //memset(command, '\0', 41);

  if(!HC05_COM("01041"))
    return -1;


  if(buffer[6] == '4' && buffer[7] == '1')
  {
    for(i = 12; i < 14; i++)
    {
      if(i == 11) i++; //There is a space between the two numbers

      if(buffer[i] >= 'A' && buffer[i] <= 'F')
        buffer[i] -= 55;
      if(buffer[i] >= '0' && buffer[i] <= '9')
        buffer[i] -= 48;
    }
    //HEX to DEC conversion
    byte B = (buffer[12] * 16) + buffer[13];

    return (B * 100) / 255; //OBD2 load formula
  }
  else
    return -1;
}

/////////////////////////////////////////////////////////////
int getRPM()
{
  //  memset(command, '\0', 41);

  if(!HC05_COM("010C1"))
    return -1;

  if(buffer[6] == '4' && buffer[7] == '1' && buffer[9] == '0' && buffer[10] == 'C')
  {
    for(i = 12; i < 17; i++)
    {
      if(i == 14) i++; //There is a space between the two numbers

      if(buffer[i] >= 'A' && buffer[i] <= 'F')
        buffer[i] -= 55;
      if(buffer[i] >= '0' && buffer[i] <= '9')
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


float get12Volt()
{
  if(!HC05_COM("ATRV"))
    return -1;

  for(i = 5; i < 9; i++)
  { 
    if(buffer[i] >= '0' && buffer[i] <= '9')
      buffer[i] -= 48;
  }
  
  
  float t = (float) buffer[8] / 10;
  
  return (buffer[5] * 10) + buffer[6] + t;
}





