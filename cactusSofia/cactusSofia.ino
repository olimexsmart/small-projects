/*
  This is can be useful for any ESP8266 that controls an RGB light of any sort

  For now it is just random colors, then it will be controlled via WiFi
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "credentials.h"

#define SPEED 800
#define UPDATE_FREQ 20
#define PWM_RES 1023

#define RED D8
#define GREEN D5
#define BLUE D6


int r, g, b;
int  randR, randG, randB;
bool dirR, dirG, dirB;
unsigned long lastUpdate;


void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);

  Serial.begin(115200);
  delay(10);

  /*
      Serial.println();
      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(ssid);

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
  
  // Initialize global vars
  randomSeed(analogRead(A0));
  r = random(PWM_RES);
  g = random(PWM_RES);
  b = random(PWM_RES);
  randR = random(SPEED);
  randG = random(SPEED);
  randB = random(SPEED);
  dirR = random(1);
  dirG = random(1);
  dirB = random(1);

  lastUpdate = millis();
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


  if (lastUpdate + UPDATE_FREQ < millis()) {
    backgroundUpdate(); // Update global vars
    applyRGB(); // Apply global vars

    lastUpdate = millis();
  }

}


/* #########################################################################################
   FUNCTIONS
*/

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
  r = constrain(r, 0, PWM_RES);
  g = constrain(g, 0, PWM_RES);
  b = constrain(b, 0, PWM_RES);
}

// Appling RGB values to PWM
void applyRGB() {
  analogWrite(RED, r);
  analogWrite(GREEN, g);
  analogWrite(BLUE, b);
}
