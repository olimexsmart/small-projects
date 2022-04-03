void setupFunc() {
  Ethernet.init(chipSelectEth);
  pinMode(resetEth, OUTPUT);
  pinMode(watchDog, OUTPUT);
  pinMode(opening, OUTPUT);
  digitalWrite(opening, HIGH); // Closing contact immediately
  pinMode(ledClient, OUTPUT);
  digitalWrite(ledClient, LOW);
  pinMode(chipSelectSD, OUTPUT); // Avoid conflict on SPI MISO
  digitalWrite(chipSelectSD, HIGH);
  digitalWrite(resetEth, LOW);
  delay(100);
  digitalWrite(resetEth, HIGH);
  delay(100);

  Ethernet.begin(mac, ip);  // initialize Ethernet device
  server.begin();           // start to listen for clients
  Serial.begin(9600);       // for debugging

  if (Ethernet.hardwareStatus() == EthernetW5500) {
    Serial.print(F("SUCCESS - W5500 Ethernet controller detected. Server is at: "));
    Serial.println(Ethernet.localIP());
  } else {
    Serial.println(F("ERROR - W5500 not correctly detected"));
  }
}
