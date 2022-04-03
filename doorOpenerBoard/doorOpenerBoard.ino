/*
    Server Door Opener

    TODO:
    - See access list on web page
    - There is about half an hour of error in the epoch conversion

*/
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include "HTTPparser.h"

#define chipSelectSD 9
#define chipSelectEth 10
#define ledClient 5
#define resetEth 6
#define opening 7
#define watchDog 8

#define AUTORESET 86400000 // Reset once a day
#define OPEN_DURATION 1000


// MAC address
byte mac[] = { 0x02, 0x42, 0xB5, 0x44, 0x17, 0x98 };

IPAddress ip(192, 168, 1, 33); // IP address
EthernetServer server(80);  // Create a server at port 80
EthernetClient client;
HTTPparser Parser;

// Open door flag, the opening is blocking (very simple)
// needs to be done after the client has been served
bool openDoorFlag = false;


void setup()
{
  setupFunc();
}

void loop()
{
  loopFunc();
}
