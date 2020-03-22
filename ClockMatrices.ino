// Program to demonstrate the MD_Parola library
//
// Simplest program that does something useful - Hello World!
//
// MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 4

// For regular Arduino
//#define CLK_PIN   13 
//#define DATA_PIN  11
//#define CS_PIN    10

// For ESP8266 NodeMCU
#define CLK_PIN   D5 // or SCK
#define DATA_PIN  D7 // or MOSI
#define CS_PIN    D8 // or SS

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup(void)
{
    P.begin();
    // P.displayText("Hello", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    //Serial.begin(57600);
    //Serial.println(P.getSpeed()); // defualt at 10
}

void loop(void)
{
    P.displayText("Hello", PA_CENTER, P.getSpeed(), 500, PA_OPENING_CURSOR, PA_OPENING_CURSOR);
    while(!P.displayAnimate());
    delay(1000);
}
