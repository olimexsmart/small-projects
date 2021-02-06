//
//
//
//
// MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "secrets.h"

ESP8266WiFiMulti WiFiMulti;

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 4


// For ESP8266 NodeMCU
#define CLK_PIN   D5 // or SCK
#define DATA_PIN  D7 // or MOSI
#define CS_PIN    D8 // or SS

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Global data
struct sCatalog
{
  textEffect_t  effect;   // text effect to display
  const char *  psz;      // text string nul terminated
  uint16_t      speed;    // speed multiplier of library default
  uint16_t      pause;    // pause multiplier for library default
};

sCatalog catalog[] =
{
  { PA_PRINT, "PRINT", 1, 1 },
  { PA_SCROLL_UP, "SC_U", 5, 1 },
  { PA_SCROLL_DOWN, "SCR_D", 5, 1 },
  { PA_SCROLL_LEFT, "SCR_L", 5, 1 },
  { PA_SCROLL_RIGHT, "SCR_R", 5, 1 },
  { PA_SPRITE, "SPRIT", 5, 1 },
  { PA_SLICE, "SLICE", 1, 1 },
  { PA_MESH, "MESH", 20, 1 },
  { PA_FADE, "FADE", 20, 1 },
  { PA_DISSOLVE, "DSLVE", 7, 1 },
  { PA_BLINDS, "BLIND", 7, 1 },
  { PA_RANDOM, "RAND", 3, 1 },
  { PA_WIPE, "WIPE", 5, 1 },
  { PA_WIPE_CURSOR, "WPE_C", 4, 1 },
  { PA_SCAN_HORIZ, "SCNH", 4, 1 },
  { PA_SCAN_HORIZX, "SCNHX", 4, 1 },
  { PA_SCAN_VERT, "SCNV", 3, 1 },
  { PA_SCAN_VERTX, "SCNVX", 3, 1 },
  { PA_OPENING, "OPEN", 3, 1 },
  { PA_OPENING_CURSOR, "OPN_C", 4, 1 },
  { PA_CLOSING, "CLOSE", 3, 1 },
  { PA_CLOSING_CURSOR, "CLS_C", 4, 1 },
  { PA_SCROLL_UP_LEFT, "SCR_UL", 7, 1 },
  { PA_SCROLL_UP_RIGHT, "SCR_UR", 7, 1 },
  { PA_SCROLL_DOWN_LEFT, "SCR_DL", 7, 1 },
  { PA_SCROLL_DOWN_RIGHT, "SCR_DR", 7, 1 },
  { PA_GROW_UP, "GRW_U", 7, 1 },
  { PA_GROW_DOWN, "GRW_D", 7, 1 },
};

// Sprite Definitions
const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
static const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};

const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
static const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
};

#define N_LAYERS 2
#define BUF_LEN 200
#define REQ_DELAY 10000
char payloadBuffer[BUF_LEN];
int k = 0;
char *msgBuff[] = { // Filled with some overhead. But this is wrong. FIXME
  "FormazioneXX", "{} giorni {} ore {} minutiXXX"
};

unsigned long deltaR;

void setup(void)
{
  P.begin();
  //P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  P.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);
  for (uint8_t i = 0; i < ARRAY_SIZE(catalog); i++)
  {
    catalog[i].speed *= P.getSpeed();
    catalog[i].pause *= 500;
  }

  Serial.begin(115200);
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Soffitta", "maBelin!");

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  deltaR = millis();
}

void loop(void)
{
  // GET DATA SECTION
  if ((WiFiMulti.run() == WL_CONNECTED) && millis() - deltaR > REQ_DELAY) { // wait for WiFi connection
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.1.44/downloads/dueFormazza.txt")) {  // HTTP
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);

          // Dividing string in tokens
          strcpy(payloadBuffer, payload.c_str());
          tokenize(payloadBuffer);

          for (byte i = 0; i < N_LAYERS; i++)
            Serial.println(msgBuff[i]);

        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }

    deltaR = millis();
  }



  // DISPLAY SECTION
  static textPosition_t just = PA_LEFT;
  static uint8_t i = 0;   // text effect index
  static uint8_t j = 0;   // text justification index

  if (P.displayAnimate()) // animates and returns true when an animation is completed
  {
    if (k == 0) { // Keep it casual only for first word
      // progress the justification if needed
      if (i == ARRAY_SIZE(catalog))
      {
        j++;
        if (j == 3) j = 0;

        switch (j)
        {
          case 0: just = PA_LEFT;    break;
          case 1: just = PA_CENTER;  break;
          case 2: just = PA_RIGHT;   break;
        }

        i = 0;  // reset loop index
      }

      // set up new animation
      P.displayText(msgBuff[k], just, catalog[i].speed, catalog[i].pause, catalog[i].effect, catalog[i].effect);
      i++;   // then set up for next text effect
    } else {
      P.displayText(msgBuff[k], PA_LEFT, 40, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }

    // Move to next writing
    k++;
    k %= N_LAYERS;
  }
}


void tokenize(char* line)
{
  char * pch = strtok(line, "+");
  byte i = 0;
  while (pch != NULL) {
    strcpy(msgBuff[i], pch);
    i++;
    if (i > N_LAYERS)
      break;

    pch = strtok(NULL, "+");
  }
}
