#include "Arduino.h"
#include <stdio.h>
#include <SPI.h>
#include <dw3000.h>

#define APP_NAME "READ DEV ID\r\n"
#define SPI_MOSI 10
#define SPI_MISO 9
#define SPI_SCK 8
#define SPI_CS 0
#define PIN_IRQ 2
#define PIN_RST 1

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
  pinMode(PIN_IRQ, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_IRQ), dwt_isr, RISING);
  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(0);

  // Initialize SPI
  // SPI.begin();
  // pinMode(SPI_CS, OUTPUT);
  // digitalWrite(SPI_CS, HIGH);
}

void loop() {

}




