/*
    This sketch runs on the sign.

    It tr

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "credentials.h"

#define SPEED 500
#define RED D8
#define GREEN D5
#define BLUE D6



//const char* url = "http://olimexsmart.it/embe/getCurrentHR.php";
const char* url = "http://192.168.1.44/embe/getCurrentHR.php";

bool flag = false;
byte HR = 0;
unsigned long minute, lastBeat;
unsigned int pause;
int r, g, b;
int  randR, randG, randB;
bool dirR, dirG, dirB;


void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);

  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  /*
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
*/
  // Initialize vars
  randomSeed(analogRead(A0));
  minute = millis();
  lastBeat = millis();
  pause = 1000;
  r = random(1023);
  g = random(1023);
  b = random(1023);
  randR = random(SPEED);
  randG = random(SPEED);
  randB = random(SPEED);
  dirR = random(1);
  dirG = random(1);
  dirB = random(1);
}



void loop() {
  /*
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin(url);  //Specify request destination
    int httpCode = http.GET(); //Send the request
    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      HR = (byte) payload.toInt();
      Serial.println("HR: " + payload);  //Print the response payload
    }
    http.end();   //Close connection
  }
*/
  while (millis() - minute < 60000) { // Cycle inside for a minute

    backgroundUpdate(); // Update global vars and apply
    applyRGB(0);

    // Beat only if there is a HR data, 0 means no data
    if (millis() - lastBeat > pause && HR > 0) { // HR beat correct intervals
      heartBeat(1023); // For the moment always at max intensity
      lastBeat = millis();
      pause = 60000 / HR;
    }

    delay(10); // Don't change background too fast
  }

  minute = millis();
}


/* #########################################################################################
   FUNCTIONS
*/
// Applies over current RGB combination a HR cycle, saturating it to white
void heartBeat(int intensity) {
  // Definting different steps in the beat sequence
  int midway = 2 * (intensity / 3);
  int lowend = intensity / 10;

  // Increasing
  for (int i = 0; i < intensity; i++) {
    applyRGB(i);
    delayMicroseconds(100);
  }

  // Decreasing
  for (int i = intensity; i > lowend; i--) {
    applyRGB(i);
    delayMicroseconds(100);
  }

  delay(25); // Pause between contractions
  
  // Increasing
  for (int i = lowend; i < midway; i++) {
    applyRGB(i);
    delayMicroseconds(150);
  }

  // Decreasing
  for (int i = midway; i > lowend; i--) {
    applyRGB(i);
    delayMicroseconds(750);
  }
  for (int i = lowend; i > 0; i--) {
    applyRGB(i);
    delayMicroseconds(1500);
  }

}

// Trying to linearize our perception of light intensity with the 1023 levels of intensity
int deLog(int x) {
  return ((511500.0) / ((x * 0.671) - 1023.0)) + 1523.0;
}

// Random backgroung update
void backgroundUpdate() {
  // Direction change whenever a there is a random match
  if (randR == random(SPEED)) {
    dirR = !dirR;
    randR = random(SPEED);
  }
  if (randG == random(SPEED)) {
    dirG = !dirG;
    randG = random(SPEED);
  }
  if (randB == random(SPEED)) {
    dirB = !dirB;
    randB = random(SPEED);
  }

  // Valuea update according to direction
  r = dirR ? (r + 1) : (r - 1);
  g = dirG ? (g + 1) : (g - 1);
  b = dirB ? (b + 1) : (b - 1);
  r = constrain(r, 0, 1023);
  g = constrain(g, 0, 1023);
  b = constrain(b, 0, 1023);
}

// Appling RGB values to PWM 
void applyRGB(int bias) {
  analogWrite(RED, deLog(constrain(r + bias, 0, 1023)));
  analogWrite(GREEN, deLog(constrain(g + bias, 0, 1023)));
  analogWrite(BLUE, deLog(constrain(b + bias, 0, 1023)));
}
