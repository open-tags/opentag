#include <Adafruit_NeoPixel.h>

#define LED_PIN     6
#define NUM_LEDS    1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(50);  // Set brightness to 50/255 (about 20%)
  strip.show();
}

void loop() {
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    strip.rainbow(firstPixelHue);
    strip.show();
    delay(50);  // Reduced delay for faster color transition
  }
}