#include "Arduino.h"
#include <stdio.h>
#include <dw3000.h>
extern "C" {
  #include <niq.h>
}

#include "Adafruit_nRFCrypto.h"
#include <BLEService.h>
#include <BLECharacteristic.h>
#include <bluefruit.h>

#define APP_NAME "READ DEV ID\r\n"
#define SPI_MOSI 10
#define SPI_MISO 9
#define SPI_SCK 8
#define SPI_CS 0
#define RST 1 
#define IRQ 2

// DWM-3000 SPI commands
#define DWM3000_DEVID_REG 0x00

// BLEService customService("180C"); // Custom BLE Service
// BLECharacteristic txCharacteristic("2A56", BLEWrite, 20); // Tx Characteristic
// BLECharacteristic rxCharacteristic("2A57", BLERead | BLENotify, 20); // Rx Characteristic

// BLE Service
BLEService myService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLEService ANIService("48fe3e40-0817-4bb2-8633-3073689c2dba");

static uint8_t accessory_config_data_buffer[sizeof(struct AccessoryConfigurationData)];
static uint8_t accessory_config_data_length;

fira_device_configure_t fira_config;

// BLE Characteristic for Tx
BLECharacteristic myTxChar("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLENotify | BLEIndicate, 38, true);
BLECharacteristic myRxChar("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLERead);

BLECharacteristic ACD("95e8d9d5-d8ef-4721-9a4e-807375f53328", BLEWrite);

void onDataWritten(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len);

static void send_ble_data(uint8_t * buffer, uint16_t data_len){
  Serial.print("Length before calling: ");
  Serial.println(data_len);
  Serial.print("MaxLen is: ");
  Serial.println(myTxChar.getMaxLen());
  bool success = myTxChar.notify(buffer, data_len);
  if (success) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Failed to send data");
  }
}

// void send_ble_data(uint8_t* buffer, uint16_t data_len) {
//     bool success = true;
//     uint16_t offset = 0;

//     while (offset < data_len) {
//         uint16_t chunk_len = min(20, data_len - offset);
//         bool chunk_success = myTxChar.notify(buffer + offset, chunk_len);
//         if (!chunk_success) {
//             success = false;
//             break;
//         }
//         offset += chunk_len;
//         delay(10); // Small delay to ensure BLE stack has time to process
//     }

//     return;
// }


void print_buffer_binary(uint8_t* buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        for (int8_t j = 7; j >= 0; --j) {
            Serial.print((buffer[i] >> j) & 1);
        }
        Serial.print(" ");
    }
    Serial.println();
}

static void send_accessory_config_data() {

    niq_reinit();

    // The Accessory Configuration Data in intended to be constructed by the embedded application.

    // The message structure is the following:
    // ------------------------
    //     majorVersion         -- the major version from section 3.3 of the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     minorVersion         -- the minor version from section 3.3 of the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     preferredUpdateRate  -- a selection of one of the options from table 3-3 in the Nearby Interaction Accessory Protocol Specification, Developer Preview, Release 1.
    //     rfu[10]              -- reserved for future use.
    //     uwbConfigDataLength  -- the length of the UWB config data as provided by the UWB middleware.
    //     uwbConfigData        -- the UWB config data as provided by the UWB middleware, according to section 4.4.2 on the UWB Interoperability Specification, Developer Preview, Release 1.
    // ------------------------
    //
    // In order to populate `uwbConfigData` and `uwbConfigDataLength`, the embedded appliaction needs to query
    // the UWB middleware which is compliant with the UWB Interoperability Specification.
    //
    // Once the Accessory Configuration Data is constucted and populated, the embedded application needs to send it to the iOS app.

    uint8_t buffer[sizeof(struct AccessoryConfigurationData)+1]; // + 1 for the MessageId
    memset(buffer, 0, sizeof(struct AccessoryConfigurationData)+1);

    ni_packet_t * packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryConfigurationData;

    // Embeded appliaction responsibility
    struct AccessoryConfigurationData *config = (struct AccessoryConfigurationData *)packet->payload;
    config->majorVersion = NI_ACCESSORY_PROTOCOL_SPEC_MAJOR_VERSION;
    config->minorVersion = NI_ACCESSORY_PROTOCOL_SPEC_MINOR_VERSION;
    config->preferredUpdateRate = PreferredUpdateRate_Automatic;
    
    // UWB middleware responsibility
    niq_populate_accessory_uwb_config_data(&config->uwbConfigData, &config->uwbConfigDataLength);

    //set_accessory_uwb_config_data(packet->payloxad);
    Serial.println((uint8_t)config->uwbConfigDataLength);

    Serial.println("Buffer in binary format (little-endian):");

    // Print the message ID
    Serial.println("Message ID");
    print_buffer_binary(&packet->message_id, 1);

    // Print the rest of the buffer in little-endian format
    Serial.println("Major Version");
    uint8_t* data_ptr = (uint8_t*)&config->majorVersion;
    for (int i = 0; i < sizeof(config->majorVersion); ++i) {
        print_buffer_binary(&data_ptr[i], 1);
    }

    Serial.println("Minor Version");
    data_ptr = (uint8_t*)&config->minorVersion;
    for (int i = 0; i < sizeof(config->minorVersion); ++i) {
        print_buffer_binary(&data_ptr[i], 1);
    }


    Serial.println("PUR");
    data_ptr = &config->preferredUpdateRate;
    print_buffer_binary(data_ptr, 1);

    // for (int i = 0; i < sizeof(config->rfu); ++i) {
    //     print_buffer_binary(&config->rfu[i], 1);
    // }

    Serial.println("uwbConfigDataLength");
    data_ptr = &config->uwbConfigDataLength;
    print_buffer_binary(data_ptr, 1);

    Serial.println("uwbConfigData");
    for (int i = 0; i < config->uwbConfigDataLength; ++i) {
        print_buffer_binary(&config->uwbConfigData[i], 1);
    }

    send_ble_data(buffer, (uint16_t)(config->uwbConfigDataLength + ACCESSORY_CONFIGURATION_DATA_FIX_LEN + 1));
}

/*
 * @brief callback used for niq lib to signal session start
 */
void ResumeUwbTasks(void)
{
    Serial.println("Started");
}

/*
 * @brief callback used for niq lib to signal session stop
*/
void StopUwbTask(void)
{
    Serial.println("Finished");
}

// Wrapper for nrf_crypto_init
void nrf_crypto_init_wrapper(void) {
    bool result = nRFCrypto.begin();
    if (!result) {
        Serial.println("Crypto init failed with error");
    }
}

// Wrapper for nrf_crypto_uninit
void nrf_crypto_uninit_wrapper(void) {
  nRFCrypto.end();
}

void nrf_crypto_rng_vector_generate_wrapper(uint8_t * const p_target, size_t size){
  bool result = nRFCrypto.Random.generate(p_target, size);
    if (!result) {
        Serial.println("Crypto generate failed with error");
    }
}

//**************************************************************************************************************
// callback function
//*******************************************************************************************************
void connect_callback(uint16_t conn_handle)
{
  Serial.print("【connect_callback】 conn_Handle : ");
  Serial.println(conn_handle, HEX);
//  conn = conn_handle;

  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  Serial.println();
  // request PHY changed to 2MB (2Mbit/sec moduration) 1 --> 2
  Serial.print("Request to change PHY : "); Serial.print(connection->getPHY());
  connection->requestPHY();
  delayMicroseconds(1000000);  // delay a bit for all the request to complete
  Serial.print(" --> "); Serial.println(connection->getPHY());

  // request to update data length  27 --> 251
  Serial.print("Request to change Data Length : "); Serial.print(connection->getDataLength());
  connection->requestDataLengthUpdate();
  delayMicroseconds(1000000);  // delay a bit for all the request to complete
  Serial.print(" --> "); Serial.println(connection->getDataLength());
    
  // request mtu exchange 23 --> 247
  Serial.print("Request to change MTU : "); Serial.print(connection->getMtu());
  connection->requestMtuExchange(247);  // max 247
//  connection->requestMtuExchange(123);  // max 247
  delayMicroseconds(1000000);  // delay a bit for all the request to complete
  Serial.print(" --> "); Serial.println(connection->getMtu());

  // request connection interval  16(20mS) --> 16(20mS)
  Serial.print("Request to change Interval : "); Serial.print(connection->getConnectionInterval());
//  connection->requestConnectionParameter(16); // 20mS(in unit of 1.25) default 20mS
  delayMicroseconds(1000000);  // delay a bit for all the request to complete
  Serial.print(" --> "); Serial.println(connection->getConnectionInterval());

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("【connect_callback】 Connected to ");
  Serial.println(central_name);
}

//***********************************************************************************************
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.print("【disconnect_callback】 reason = 0x");
  Serial.println(reason, HEX);
}

static void send_ack_uwb_started(void)
{
      // Send "UWB did start" BLE message to the app
      uint8_t buffer[1];
      ni_packet_t * packet = (ni_packet_t *)buffer;
      packet->message_id = (uint8_t)MessageId_accessoryUwbDidStart;
      send_ble_data(buffer, 1);
}

void onDataWritten(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  // This callback function is called when data is written to the characteristic
  
  // Convert the received data to a string
  // String rxData = "";
  // for (int i = 0; i < len; i++) {
  //   rxData += (char)data[i];
  // }
  
  // Serial.print("Received data: ");
  // Serial.println(rxData);

  // // Send a response back
  // String txData = "Data received: " + rxData;
  // myTxChar.notify(txData.c_str(), txData.length());
  Serial.println("Hello");
  ni_packet_t * packet = (ni_packet_t *)data;
  switch (packet->message_id) {
    case MessageId_init: {
      Serial.println("Requesting Data");
      send_accessory_config_data();
      break;
    }
    case MessageId_configure_and_start: {
      Serial.println("Configure and start");
      int ret;
      ret = niq_configure_and_start_uwb(packet->payload, len-1, (void*)&fira_config);

      switch(ret)
      {
      case -E_NIQ_INPVAL:
            Serial.println("Data len wrong");
            break;
      case -E_NIQ_VERSIONNOTSUPPORTED:
            Serial.println("Protocol version not supported");
            break;
      default:
          send_ack_uwb_started();
          break;
      }

      break;
    }
    default: {
      Serial.println("Not Requesting Data");
      break;
    }
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Initialize pins
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_SCK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IRQ), dwt_isr, RISING);
  spiBegin(2, 1);
  spiSelect(0);

  // Initialize UWB and Bluefruit
  niq_init(ResumeUwbTasks, StopUwbTask, nrf_crypto_init_wrapper, nrf_crypto_uninit_wrapper, nrf_crypto_rng_vector_generate_wrapper);
  niq_set_ranging_role(0);

  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  Bluefruit.begin(1,0);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Initialize the first service and characteristic
  myService.begin();
  myTxChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  myTxChar.begin();
  myTxChar.setWriteCallback(onDataWritten);

  // // Prepare accessory data
  // niq_populate_accessory_uwb_config_data(accessory_config_data_buffer, &accessory_config_data_length);
  // accessory_config_data_length += ACCESSORY_CONFIGURATION_DATA_FIX_LEN;

  // // Initialize the second service and characteristic
  // ANIService.begin();
  // ACD.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  // // ACD.setFixedLen(accessory_config_data_length);
  // // ACD.setBuffer(accessory_config_data_buffer, (uint16_t)accessory_config_data_length);
  // ACD.begin();
  // ACD.setWriteCallback(onDataWritten);

  // Start advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(myService);
  //Bluefruit.Advertising.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
  Serial.println("Waiting for Connection");
}

void loop() {
  // if (dwt_check_dev_id() == DWT_SUCCESS) {
  //   Serial.println("DEV ID OK\n");
  // } else {
  //   Serial.println("DEV ID FAILED\n");
  // }
  // delay(10000);
}

