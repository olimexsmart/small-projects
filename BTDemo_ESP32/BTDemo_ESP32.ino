#include "WiFi.h"
#include "BLE2902.h"
#include <U8x8lib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Preferences.h>

#define STR_DATA_LEN 200
#define PIN_LED 35
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

Preferences preferences;
char strData[STR_DATA_LEN];
char strDataRead[STR_DATA_LEN];
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool notifyEnable = false;
int lineCounter = 0;
int notifyCounter = 0;
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/18, /* data=*/17, /* reset=*/21);
BLEServer *pServer;
BLECharacteristic *pCharacteristic;

void writeLineOnScreen(const char *s) {
  if (lineCounter >= u8x8.getRows()) {
    lineCounter = 0;
    u8x8.clearDisplay();
  }
  u8x8.drawString(0, lineCounter, s);
  lineCounter++;
}

void resetScreen() {
  u8x8.clearDisplay();
  lineCounter = 0;
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device CONNECTED");
    resetScreen();
    writeLineOnScreen("CONNECTED");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device DISCONNECTED");
    writeLineOnScreen("DISCONNECTED");
  };
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String val = pCharacteristic->getValue();
    if (val.length() > 0) {
      Serial.print("onWrite: ");
      for (int i = 0; i < val.length(); i++)
        Serial.print(val[i]);
      Serial.println();
      // Check for notification enable
      if (val == "NOTIFY_ON") {
        notifyEnable = true;
      } else if (val == "NOTIFY_OFF") {
        notifyEnable = false;
      } else {
        // Saving received value for the read
        memset(strData, '\0', STR_DATA_LEN);
        strncpy(strData, val.c_str(), STR_DATA_LEN);
      }
      writeLineOnScreen(val.c_str());
      // preferences.putString("strData", strData); // This saves data (not used)
    }
  }

  void onRead(BLECharacteristic *pCharacteristic) {
    sprintf(strDataRead, "%s_READ_BACK", strData);
    pCharacteristic->setValue((uint8_t *)strDataRead, strlen(strDataRead));
    writeLineOnScreen("READ");
  }

  void onNotify(BLECharacteristic *pCharacteristic) {
    String val = pCharacteristic->getValue();
    Serial.println("onNotify");
    for (int i = 0; i < val.length(); i++)
      Serial.print(val[i]);
    Serial.println();
  }
};


void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
  Serial.println("SETUP START");

  // preferences.begin("my-app", false);
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  memset(strData, '\0', STR_DATA_LEN);
  memset(strDataRead, '\0', STR_DATA_LEN);
  // preferences.getString("strData", strData, STR_DATA_LEN);
  // u8x8.drawString(0, 4, strData);
  // Serial.println(strData);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_OFF);
  WiFi.disconnect();
  delay(100);



  BLEDevice::init("MyESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ
      | BLECharacteristic::PROPERTY_WRITE
      | BLECharacteristic::PROPERTY_NOTIFY);
  // | BLECharacteristic::PROPERTY_INDICATE

  // Add descriptor to enable notifications
  BLE2902 *p2902 = new BLE2902();
  p2902->setNotifications(true);
  pCharacteristic->addDescriptor(p2902);

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  // BLEAdvertising *pAdvertising = pServer->getAdvertising();
  // pAdvertising->start();
  pServer->startAdvertising();


  Serial.println("SETUP END");
}


void loop() {
  // notify changed value
  if (deviceConnected && notifyEnable) {
    notifyCounter++;
    sprintf(strDataRead, "%s_N_%i", strData, notifyCounter);
    pCharacteristic->setValue((uint8_t *)&strDataRead, strlen(strDataRead));
    pCharacteristic->notify();
    delay(1000);
    writeLineOnScreen(strDataRead);
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  digitalWrite(PIN_LED, deviceConnected);
}




// static void doSomeWork() {
//   // int n = WiFi.scanNetworks();

//   // if (n == 0) {
//   //   u8x8.drawString(0, 0, "Searching networks.");
//   // } else {
//   //   u8x8.drawString(0, 0, "Networks found: ");
//   //   for (int i = 0; i < n; ++i) {
//   //     // Print SSID for each network found
//   //     char currentSSID[64];
//   //     WiFi.SSID(i).toCharArray(currentSSID, 64);
//   //     u8x8.drawString(0, i + 1, currentSSID);
//   //   }
//   // }

//   // Wait a bit before scanning again
//   delay(5000);
//   digitalWrite(PIN_LED, !digitalRead(PIN_LED));
// }