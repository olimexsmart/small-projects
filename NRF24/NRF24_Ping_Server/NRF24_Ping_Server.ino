/**
 * An Mirf example which copies back the data it recives.
 *
 * Pins:
 * Hardware SPI:
 * MISO -> 12
 * MOSI -> 11
 * SCK -> 13
 *
 * Configurable:
 * CE -> 8
 * CSN -> 7
 *
 * Mirf.configRegister(RF_SETUP,0x26); //250k
 * Mirf.configRegister(RF_SETUP,0x06); //1mb
 * Mirf.configRegister(RF_SETUP,0x0E); //2mb
 */

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

void setup(){
  Serial.begin(9600);

  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"serv1");
  Mirf.payload = sizeof(unsigned long);
  Mirf.config();
  Mirf.configRegister(RF_SETUP,0x26);
  
  Serial.println("Listening..."); 
}

void loop(){
  byte data[Mirf.payload];
   
  if(!Mirf.isSending() && Mirf.dataReady()){
    Serial.println("Got packet");
     
    Mirf.getData(data);
    Mirf.setTADDR((byte *)"clie1");
    Mirf.send(data);
    
    Serial.println("Reply sent.");
  }
}
