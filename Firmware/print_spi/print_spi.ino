#include <SPI.h>

void setup() {
  Serial.begin(115200);
  
  Serial.println("SPI pin configuration on ESP32:");
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);
}

void loop() {
  // Empty loop
}