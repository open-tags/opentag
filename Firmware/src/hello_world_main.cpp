#include "Arduino.h"
#include <stdio.h>
#include <SPI.h>
#include <dw3000.h>

#define APP_NAME "READ DEV ID\r\n"
#define SPI_MOSI 10
#define SPI_MISO 9
#define SPI_SCK 8
#define SPI_CS 0
#define IRQ 2

// DWM-3000 SPI commands
#define DWM3000_DEVID_REG 0x00

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_SCK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  pinMode(IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IRQ), dwt_isr, RISING);
  spiBegin(2, 1);
  spiSelect(0);

  // Initialize SPI
  // SPI.begin();
  // pinMode(SPI_CS, OUTPUT);
  // digitalWrite(SPI_CS, HIGH);
}

void loop() {
  // Read the DEVID of the DWM-3000 chip
  // uint32_t devid = readDWM3000Register(DWM3000_DEVID_REG, 4);

  // // Convert the DEVID to characters and print
  // Serial.print("DWM-3000 DEVID Characters: ");
  // for (int i = 3; i >= 0; i--) {
  //   char c = (char)(devid >> (i * 8) & 0xFF); // Extract each byte
  //   if (c >= 32 && c <= 126) { // Check if the character is printable
  //     Serial.print(c);
  //   } else {
  //     Serial.print("."); // Print a placeholder for non-printable characters
  //   }
  // }
  // Serial.println();

  // // Optionally, print the DEVID in hexadecimal format
  // Serial.print("DWM-3000 DEVID Hex: 0x");
  // Serial.println(devid, HEX);

  // delay(1000); // Wait for 1 second before reading again
  if (dwt_check_dev_id() == DWT_SUCCESS) {
    Serial.println("DEV ID OK\n");
  } else {
    Serial.println("DEV ID FAILED\n");
  }
  delay(10000);

}




