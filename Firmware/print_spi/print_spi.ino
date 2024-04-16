#include <SPI.h>

constexpr byte GEN_CFG_AES_0 = 0x01;
constexpr byte DEV_ID = 0x00;

void setup() {
  USBSerial.begin(115200);
  
  USBSerial.println("SPI pin configuration on ESP32:");
  USBSerial.print("MOSI: ");
  USBSerial.println(MOSI);
  USBSerial.print("MISO: ");
  USBSerial.println(MISO);
  USBSerial.print("SCK: ");
  USBSerial.println(SCK);
  USBSerial.print("SS: ");
  USBSerial.println(SS);
}

void loop() {
  uint32_t devID = read32bitReg(GEN_CFG_AES_0, DEV_ID);
    if (devID != 0xDECA0302 && devID != 0xDECA0312) {
        return;
    }
  delay(10000);
}

uint32_t read32bitReg(byte address, int offset) {
    uint32_t value = 0;
    byte buffer[4];

    readReg(address, offset, 4, buffer);
    for (int i = 3; i >= 0; i--) {
        value = (value << 8) + buffer[i];
    }

    return value;
}