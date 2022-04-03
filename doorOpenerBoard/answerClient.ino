/*
    This function takes no arguments because it' just for code readability.
    This code would be too long to be directly written in the main loop.
*/

void answerClient() {
  switch (Parser.Method) {
    case HTTPparser::GET:
      if (strcmp(Parser.Path, "/open") == 0) { // Door openings count
        sendHeaders(200, "text/html");
        openDoorFlag = true;
        client.print("OPENED");
        break;
      }
    default:
      // 501 not implemented
      sendHeaders(501, NULL);
      break; // Break from default
  }
}


// Constant arguments declared only in the prototype
void sendHeaders(int code, const char * mime) {
  client.print(F("HTTP/1.1 "));
  // Sent some HTTP headers
  switch (code) {
    case 200:
      client.println(F("200 OK"));
      break;
    case 403:
      client.println(F("403 Forbidden"));
      break;
    case 404:
      client.println(F("404 Not Found"));
      break;
    case 418:
      client.println(F("418 I'm a teapot"));
      break;
    case 423:
      client.println(F("423 Locked"));
      break;
    case 429:
      client.println(F("429 Too Many Requests"));
      break;
    case 501:
      client.println(F("501 Not Implemented"));
      break;
    case 500:
      client.println(F("500 Internal Server Error"));
      break;
  }
  // Append correct image format
  if (mime) {
    client.print(F("Content-Type: "));
    client.println(mime);
  }
  client.println(F("Connection: close"));
  client.println();
}
