#include <Arduino.h>
#include <USB.h>
#include <SPI.h>


// put function declarations here:
int myFunction(int, int);

void setup() {
  USBSerial.begin(115200);

  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  USBSerial.println("Entering loop");
  delay(1000);
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}