/*
Test
 Use the HC-05 Bluetooth module over the serial port

 Thanks to Kostas Kokoras - kostas@kokoras.com
 */

#include <SoftwareSerial.h>


#define TxBT A2
#define RxBT A1
#define EnableATBT A3

SoftwareSerial SerialBT(TxBT, RxBT);
char command[41];
boolean mode;
boolean notNow = false;
unsigned long time;

//######################################################
void setup()
{
  pinMode(RxBT, OUTPUT);
  pinMode(TxBT, INPUT);
  pinMode(EnableATBT, OUTPUT);

  digitalWrite(EnableATBT, LOW);    //Set HC-05 to Com mode

  Serial.begin(9600);
  SerialBT.begin(38400);
  delay(500);
  //Setup HC-05
  HC05_SetupCOM();
  HC05_ComMode();
  mode = true;

  time = millis();
}


//###########################################################
void loop()
{
  while (Serial.available() <= 0)
  {
    if ((time < millis() - 1000) && SerialBT.available() > 0) { //check sometimes if something was received
      while (SerialBT.available() > 0)
        Serial.write(SerialBT.read());
      time = millis();
    }
  }

  Serial.println();

  Serial.readBytes(command, 40);

  for (byte i = 0; i < 40; i++)
    command[i] = toupper(command[i]);

  Serial.println(command);

  //This segment switches between the two operating modes
  if (strcasecmp(command, "COM") == 0)
  {
    HC05_ComMode();
    Serial.println("HC-05 entered in communication mode.");
    mode = false;
    notNow = true;
  }

  if (strcasecmp(command, "ATMODE") == 0)
  {
    HC05_ATMode();
    Serial.println("HC-05 entered in AT mode.");
    mode = true;
    notNow = true;
  }


  if (!notNow) {  //don't want to send switching-mode strings
    if (mode)
      HC05_ATCommand(command);
    else
      HC05_COM(command);
  }

  notNow = false;
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

void HC05_ComMode()
{
  SerialBT.flush();
  delay(250);
  digitalWrite(EnableATBT, LOW);
  delay(250);
}

void HC05_ATCommand(char *command)
{
  //Send the command
  SerialBT.print("AT");
  if (strlen(command) > 1) { //Check if it is a valid one
    SerialBT.print("+");
    SerialBT.print(command);
  }
  SerialBT.print("\r\n");

  time = millis();
  while (SerialBT.available() <= 0)
  {
    if (Serial.available() > 0)
      break;
  }  //wait while no data

  while (SerialBT.available() > 0)
  {
    Serial.write(SerialBT.read());  //Save the response
  }
}

void HC05_COM(char *command)
{
  SerialBT.print(command);
  SerialBT.print("\r");

  while (SerialBT.available() <= 0)
  {
    if (Serial.available() > 0)
      break;
  }

  delay(1500); //Time to the obd to think

  while (SerialBT.available() > 0)
  {
    Serial.write(SerialBT.read());
  }

  Serial.println();
}

void HC05_SetupCOM()
{
  Serial.println("In setup DB");
  //Bluetooth connection setup
  HC05_ATCommand("RESET");
  delay(2000);

  Serial.println("role DB");
  HC05_ATCommand("ROLE=1");
  delay(500);

  Serial.println("cmode DB");
  HC05_ATCommand("CMODE=0");
  delay(500);

  Serial.println("bind DB");
  HC05_ATCommand("BIND=19, 5D, 26A42A");
  delay(500);

  Serial.print("Initialization...");
  HC05_ATCommand("INIT");
  delay(750);
  HC05_ATCommand("PAIR=19, 5D, 26A42A,20");
  delay(500);
  Serial.print("Linking...");
  HC05_ATCommand("LINK=19, 5D, 26A42A");
  delay(1000);

  //If everything was fine, it's time to start communication

  Serial.print("Entering in COM mode...");
  HC05_ComMode();
  //OBD2 interface setup

  Serial.print("OBD2 interfacing...");
  HC05_COM("ATZ");
  delay(1500);
  HC05_COM("ATSP0");
  delay(1500);
  HC05_COM("0100");
  delay(3000);

  Serial.println("Connected to the car");
}








