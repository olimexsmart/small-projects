void loopFunc() {
  // Call this only once for entire loop
  unsigned long ora = millis();

  // Reset watchdog, but let the watchdog reset us
  if (AUTORESET > ora) {
    digitalWrite(watchDog, HIGH);
    delay(10);
    digitalWrite(watchDog, LOW);
  }

  // Door opening
  if (openDoorFlag) {
    digitalWrite(opening, LOW);
    delay(OPEN_DURATION);
    digitalWrite(opening, HIGH);
    openDoorFlag = false;
  }

  // Refresh client status, this should call the copy assignement operator generated by the compiler
  client = server.available();  // try to get client
  // Got client?
  if (client) {
    digitalWrite(ledClient, HIGH);
    while (client.connected()) {  // Collecting data from client
      if (client.available()) {   // client data available to read
        Parser.ParseChar(client.read()); // Read client one char
      } else {
        break; // Break when there is no more
      }
    }
    // Tell the parser we are done
    Parser.AllSheWrote();
    if (Parser.IsValid()) {
      // Print some debug
      Serial.println();
      Serial.println(Parser.MethodString());
      Serial.println(Parser.Path);
      Serial.println(Parser.Message);
      // Answer the client and elaborate actions
      answerClient();
    } else {
      Serial.println(F("We have an error"));
      // Error 500
      sendHeaders(500, NULL);
    }
    // Give the web browser time to receive the data
    delay(1);
    // Close the connection
    client.stop();
    // Prepare parser for new request
    Parser.Reset();
    digitalWrite(ledClient, LOW);
  }
}
