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

#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385

// DWM-3000 SPI commands
#define DWM3000_DEVID_REG 0x00

/* Frames used in the ranging process. See NOTE 3 below. */
static uint8_t tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8_t rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Length of the common part of the message (up to and including the function code, see NOTE 3 below). */
#define ALL_MSG_COMMON_LEN 10
/* Indexes to access some of the fields in the frames defined above. */
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4
/* Frame sequence number, incremented after each transmission. */
static uint8_t frame_seq_nb = 0;

/* Buffer to store received response message.
 * Its size is adjusted to longest frame that this example code is supposed to handle. */
#define RX_BUF_LEN 20
static uint8_t rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */
static uint32_t status_reg = 0;
volatile static uint8_t start_uwb = 0;

/* Delay between frames, in UWB microseconds. See NOTE 1 below. */
#ifdef RPI_BUILD
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#endif //RPI_BUILD
#ifdef STM32F429xx
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#endif //NRF52840_XXAA
/* Receive response timeout. See NOTE 5 below. */
#ifdef RPI_BUILD
#define RESP_RX_TIMEOUT_UUS 270
#endif //RPI_BUILD
#ifdef STM32F429xx
#define RESP_RX_TIMEOUT_UUS 210
#endif //STM32F429xx
#ifdef NRF52840_XXAA
#define RESP_RX_TIMEOUT_UUS 100
#endif //NRF52840_XXAA

#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#define RESP_RX_TIMEOUT_UUS 0

dwt_config_t * dwt_config;


/* Hold copies of computed time of flight and distance here for reference so that it can be examined at a debug breakpoint. */
static double tof;
static double distance;


// BLE Service
BLEService myService("26b886e0-0bab-4d96-a8f3-f44fe442a700");
BLEService ANIService("48fe3e40-0817-4bb2-8633-3073689c2dba");

static uint8_t accessory_config_data_buffer[sizeof(struct AccessoryConfigurationData)];
static uint8_t accessory_config_data_length;

fira_device_configure_t fira_config;
extern dwt_txconfig_t txconfig_options;

// BLE Characteristic for Tx
BLECharacteristic myTxChar("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLENotify | BLEIndicate, 38, true);
BLECharacteristic myRxChar("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLERead);

BLECharacteristic ACD("95e8d9d5-d8ef-4721-9a4e-807375f53328", BLERead);


static void send_ble_data(uint8_t * buffer, uint16_t data_len){
  bool success = myTxChar.notify(buffer, data_len);
  if (success) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Failed to send data");
  }
}

void print_buffer_binary(uint8_t* buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        for (int8_t j = 7; j >= 0; --j) {
            Serial.print((buffer[i] >> j) & 1);
        }
        Serial.print(" ");
    }
    Serial.println();
}

void print_binary(uint32_t value) {
    char binary_str[33]; // 32 bits + null terminator
    binary_str[32] = '\0'; // Null terminator

    for (int i = 31; i >= 0; --i) {
        binary_str[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }

    Serial.println(binary_str);
}

static void send_accessory_config_data() {

    Serial.println("Sending Config data for the first time");
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

    send_ble_data(buffer, (uint16_t)(config->uwbConfigDataLength + ACCESSORY_CONFIGURATION_DATA_FIX_LEN + 1));
    memset(buffer, 0, sizeof(struct AccessoryConfigurationData)+1);
}

void spi_rd_err_cb(){
  Serial.print("Bruhhh");
}

static void send_ack_uwb_started(void)
{
      // Send "UWB did start" BLE message to the app
      Serial.println("Sending Acknowledgement");
      uint8_t buffer[1] = {2};
      send_ble_data(buffer, 1);
}


/*
 * @brief callback used for niq lib to signal session start
 */
void ResumeUwbTasks(void)
{ 
    start_uwb = 1;
}


/*
 * @brief callback used for niq lib to signal session stop
*/
void StopUwbTask(void)
{
    start_uwb = 0;
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



static void send_ack_uwb_stopped(void)
{
    uint8_t buffer[1];
    ni_packet_t * packet = (ni_packet_t *)buffer;
    packet->message_id = (uint8_t)MessageId_accessoryUwbDidStop;
    send_ble_data(buffer, 1);
}

void onDataWritten(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  ni_packet_t * packet = (ni_packet_t *)data;
  switch (packet->message_id) {
    case MessageId_init: {
      send_accessory_config_data();
      break;
    }
    case MessageId_configure_and_start: {
      int ret;
      ret = niq_configure_and_start_uwb(packet->payload, len-1, (void*)&fira_config);

      switch(ret)
      {
      case -E_NIQ_INPVAL:
            Serial.println("ERROR: Data len wrong");
            break;
      case -E_NIQ_VERSIONNOTSUPPORTED:
            Serial.println("ERROR: Protocol version not supported");
            break;
      default:
        //Serial.println("UWB STARTED");
        send_ack_uwb_started();
        Serial.println("TWR Started");
        break;
      }

      break;
    }
    case MessageId_stop: {
      // Stop accessory UWB and reset state
      niq_stop_uwb();
      send_ack_uwb_stopped();
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
  spiBegin(IRQ, RST);
  spiSelect(SPI_CS);

  // Populate the dwt_config (Currently hardcoded)
  uint8_t sfd_id = 2;
  dwt_config->chan = 9;
  dwt_config->sfdType = (sfd_id == 0) ? DWT_SFD_IEEE_4A : DWT_SFD_IEEE_4Z; //TBD: maybe_flawed?
  //dwt_config->sfdType = 2;
  dwt_config->txCode = 11;
  dwt_config->rxCode = 11;
  dwt_config->txPreambLength = DWT_PLEN_64;
  dwt_config->rxPAC = DWT_PAC8;
  dwt_config->dataRate = DWT_BR_6M8;
  dwt_config->phrMode = DWT_PHRMODE_STD;
  dwt_config->phrRate = DWT_PHRRATE_STD;
  dwt_config->sfdTO = (65  + 8 - 8) ;
  dwt_config->stsMode = DWT_STS_MODE_OFF;
  dwt_config->stsLength = DWT_STS_LEN_64;
  dwt_config->pdoaMode = DWT_PDOA_M0;

  while (!dwt_checkidlerc()) // Need to make sure DW IC is in IDLE_RC before proceeding 
  {
    Serial.println("IDLE FAILED\r\n");
    while (1) ;
  }

  // Enab
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  if (dwt_initialise(DWT_DW_IDLE_RC) == DWT_ERROR)
  {
    Serial.println("INIT FAILED\r\n");
    while (1) ;
  }

    /* Configure DW IC. See NOTE 6 below. */
  if(dwt_configure(dwt_config)) // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
  {
    UART_puts("CONFIG FAILED\r\n");
    while (1) ;
  }

    /* Configure the TX spectrum parameters (power, PG delay and PG count) */
  dwt_configuretxrf(&txconfig_options);

  /* Apply default antenna delay value. See NOTE 2 below. */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  dwt_enablespicrccheck(DWT_SPI_CRC_MODE_NO, spi_rd_err_cb);

  // TODO: Replace code after this to switch is with Tx/Rx
  // Don't change other shit pls
  dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
  dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

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
  ANIService.begin();
  ACD.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  ACD.setFixedLen(accessory_config_data_length);
  ACD.setBuffer(accessory_config_data_buffer, (uint16_t)accessory_config_data_length);
  ACD.begin();
  // ACD.setWriteCallback(onDataWritten);

  // Start advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(myService);
  Bluefruit.Advertising.addName();

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

  while (!start_uwb){}
  Serial.println("Exited out of volatile while loop");
}

void debug(){
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

    for (int i = 0; i < sizeof(config->rfu); ++i) {
        print_buffer_binary(&config->rfu[i], 1);
    }

    Serial.println("uwbConfigDataLength");
    data_ptr = &config->uwbConfigDataLength;
    print_buffer_binary(data_ptr, 1);

    Serial.println("uwbConfigData");
    for (int i = 0; i < config->uwbConfigDataLength; ++i) {
        print_buffer_binary(&config->uwbConfigData[i], 1);
    }
    
    if (accessory_config_data_length <= sizeof(accessory_config_data_buffer))
    {
      memcpy(accessory_config_data_buffer, packet -> payload, accessory_config_data_length);
    }
    ACD.setBuffer(accessory_config_data_buffer, (uint16_t)accessory_config_data_length);

}