#include <dw3000.h>
#include "Arduino.h"
#include <stdio.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define APP_NAME "READ DEV ID\r\n"
#define SERVICE_UUID "df985029-ffb1-4fa3-a472-e0801b4543e8"
#define CHARACTERISTIC_UUID "df985029-ffb1-4fa3-a472-e0801b4543e8"

// connection pins
const uint8_t PIN_RST = 0; // reset pin
const uint8_t PIN_IRQ = 1; // irq pin
const uint8_t PIN_SS = 7; // spi select pin

BLECharacteristic *pcharacteristic;
bool deviceConnected = false;
int txValue = 0;

class tagCallBack: public BLEServerCallbacks {
  void onConnect (BLEServer *pserver){
    printf("Connected in callback\n");
    deviceConnected = true;
  };

  void onDisconnect (BLEServer *pserver){
    printf("Disconnected in callback\n");
    deviceConnected = false;
    BLEDevice::startAdvertising();
    printf("Advertising\n");
  };

};

void setup() {
  printf(APP_NAME);

  /* Configure SPI rate, DW3000 supports up to 38 MHz */
  /* Reset DW IC */
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  // Create BLE Device
  BLEDevice::init("OpenTag");

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer -> setCallbacks(new tagCallBack());

  // Create BLE Service
  BLEService *pService = pServer -> createService(SERVICE_UUID);
 
  // Setup Characteristics
  pcharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);

  // Notification setup
  pcharacteristic -> addDescriptor(new BLE2902());

  pService -> start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  printf("Characteristic defined! Now you can read it in your phone!\n");
  printf("Waiting To Connect\n");  
}

void loop() {
    /* Reads and validate device ID returns DWT_ERROR if it does not match expected else DWT_SUCCESS */
  // if (dwt_check_dev_id() == DWT_SUCCESS)
  // {
  //     printf("DEV ID OK\n");
  // }
  // else
  // {
  //     printf("DEV ID FAILED");
  // }
  if (deviceConnected){
      printf("Connected\n");
      txValue = random(-10, 20);

      char txString[8];
      dtostrf(txValue, 1, 2, txString);
      pcharacteristic -> setValue(txString);
      pcharacteristic -> notify();

      printf("Sent Value\n");
  }
  delay(1000);
}


