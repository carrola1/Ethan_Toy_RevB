#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

/* Connect NeoPixel   to digital 4
   Connect SCL        to analog 5
   Connect SDA        to analog 4
   Connect MOSI       to digital 11
   Connect SCK        to digital 13
   Connect SS         to digital 10
*/

//**********************************************
// Initialize color sensor
//**********************************************
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_4X);

//**********************************************
// Initialize neopixel
//**********************************************
#define NEO_PIN   4
#define NUMPIXELS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_RGB + NEO_KHZ800);
uint16_t audioTriggered;  // Remember audio was triggered

//**********************************************
// Initialize audio triggers
//**********************************************
#define AUD_TRIG_PIN 9

//**********************************************
// Initialize accelerometer
//**********************************************
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

//**********************************************
// Initialize LED Driver
//**********************************************
#define LED_MOSI_PIN 11
#define LED_SCK_PIN  13
#define LED_SS_PIN   10
LedControl lc=LedControl(LED_MOSI_PIN,LED_SCK_PIN,LED_SS_PIN,1);

void setup(void) {

  //**********************************************
  // Setup serial port
  //**********************************************
  Serial.begin(9600);

  //**********************************************
  // Setup color sensor
  //**********************************************
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  //**********************************************
  // Setup neopixel
  //**********************************************
  pixels.begin(); // This initializes the NeoPixel library.

  //**********************************************
  // Setup audio triggers
  //**********************************************
  pinMode(AUD_TRIG_PIN, OUTPUT);
  digitalWrite(AUD_TRIG_PIN, HIGH); 
  audioTriggered = 0;

  //**********************************************
  // Setup accelerometer
  //**********************************************
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!

  //**********************************************
  // Setup LED Driver
  //**********************************************
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  
}

void loop(void) {
  
  //********************************************** 
  // Get color sensor data
  //**********************************************
  uint16_t r, g, b, c, colorTemp, lux;
  uint16_t rScale,gScale,bScale;
  
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);

  rScale = (r >> 8);
  gScale = (g >> 8);
  bScale = (b >> 8);
  
  //Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  //Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(rScale, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(gScale, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(bScale, DEC); Serial.print(" ");
  //Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");

  //**********************************************
  // Update Neopixel color
  //**********************************************
  pixels.setPixelColor(0, pixels.Color(rScale,gScale,bScale));
  pixels.show(); // This sends the updated pixel color to the hardware.

  //**********************************************
  // Trigger audio if criteria met
  //**********************************************
  digitalWrite(AUD_TRIG_PIN, HIGH);
  if ((bScale > rScale+20) & (bScale > gScale+20)) {
    if (audioTriggered == 0) {
      digitalWrite(AUD_TRIG_PIN, LOW);
      audioTriggered = 1;
      Serial.println("Triggered!");
    }
  } else {
    audioTriggered = 0;
  }

  //**********************************************
  // Get accelerometer data
  //**********************************************
  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  Serial.print("X:  "); Serial.print(lis.x); 
  Serial.print("  \tY:  "); Serial.print(lis.y); 
  Serial.print("  \tZ:  "); Serial.print(lis.z);
  Serial.println();

  //**********************************************
  // Set LED bar graphs
  //**********************************************
  //switch on the led in the 3'rd row 8'th column
  //and remember that indices start at 0! 
  //lc.setLed(0,2,7,true);
  
  delay(100);
}
