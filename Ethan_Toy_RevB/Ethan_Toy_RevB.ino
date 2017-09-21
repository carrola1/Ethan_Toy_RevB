#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <LedControl.h>

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
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
uint8_t objFound;
uint16_t colorTimer;
#define COLOR_TO 5

//**********************************************
// Initialize neopixel
//**********************************************
#define NEO_PIN   4
#define NUMPIXELS 1
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_RGB + NEO_KHZ800);

//**********************************************
// Initialize audio triggers
//**********************************************
#define AUD_TRIG_BLUE_PIN A0
#define AUD_TRIG_RED_PIN A1
#define AUD_TRIG_GREEN_PIN A2

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
uint8_t ledCnt,ledDly;

void setup(void) {

  //**********************************************
  // Setup serial port
  //**********************************************
  Serial.begin(115200);

  //**********************************************
  // Setup color sensor
  //**********************************************
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  objFound = 0;
  colorTimer = COLOR_TO;

  //**********************************************
  // Setup neopixel
  //**********************************************
  pixels.begin(); // This initializes the NeoPixel library.

  //**********************************************
  // Setup audio triggers
  //**********************************************
  pinMode(AUD_TRIG_BLUE_PIN, OUTPUT);
  digitalWrite(AUD_TRIG_BLUE_PIN, HIGH); 
  pinMode(AUD_TRIG_RED_PIN, OUTPUT);
  digitalWrite(AUD_TRIG_RED_PIN, HIGH);
  pinMode(AUD_TRIG_GREEN_PIN, OUTPUT);
  digitalWrite(AUD_TRIG_GREEN_PIN, HIGH);

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
  ledCnt = 0;
  ledDly = 0;
}

void loop(void) {
  
  //********************************************** 
  // Get color sensor data
  //**********************************************
  uint16_t r, g, b, c, colorTemp, lux;
  double rPer,gPer,bPer,rgbTot;
  
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);

  rgbTot = double(r + b + g + 1);
  rPer = r/rgbTot*100.0;
  gPer = g/rgbTot*100.0;
  bPer = b/rgbTot*100.0;
  
  //Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  //Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(rPer, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(gPer, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(bPer, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");

  //**********************************************
  // Update Neopixel color and trigger audio
  //**********************************************
  if (colorTimer == COLOR_TO) {
    // Object Present
    if (c > 1500) { 
      if (objFound = 1) {
        ///// MOSTLY RED...options are RED, PINK, ORANGE /////
        // RED (high red, low and equal blue and green)
        if ((rPer > 55) & (abs(gPer-bPer)<=1.0)) {
          pixels.setPixelColor(0, pixels.Color(255,20,20));
          digitalWrite(AUD_TRIG_RED_PIN, LOW);
          colorTimer = 0;
          Serial.println("RED FOUND!");
        // ORANGE (high red, green slightly higher than blue)
        } else if ((rPer > 55) & (gPer > bPer) & (gPer < bPer+5)) {
          pixels.setPixelColor(0, pixels.Color(255,127,0));
          //digitalWrite(AUD_TRIG_RED_PIN, LOW);
          colorTimer = 0;
          Serial.println("ORANGE FOUND!");
        // YELLOW
        } else if ((rPer > 40) & (gPer > 35)) {
          pixels.setPixelColor(0, pixels.Color(180,255,50));
          //digitalWrite(AUD_TRIG_RED_PIN, LOW);
          colorTimer = 0;
          Serial.println("YELLOW FOUND!");
        // Pink (high red, medium and about equal blue and green)
        } else if ((rPer > 38) & (rPer < 50) & (abs(gPer-bPer)<=1.5)) {
          pixels.setPixelColor(0, pixels.Color(160,0,220));
          //digitalWrite(AUD_TRIG_RED_PIN, LOW);
          colorTimer = 0;
          Serial.println("PINK FOUND!");
        // BLUE
        } else if (bPer > 40) {
          pixels.setPixelColor(0, pixels.Color(0,50,255));
          digitalWrite(AUD_TRIG_BLUE_PIN, LOW);
          colorTimer = 0;
          Serial.println("BLUE FOUND!");
        // PURPLE (medium red and blue)
        } else if ((rPer > 34) & (bPer > 34)) {
          pixels.setPixelColor(0, pixels.Color(80,0,255));
          //digitalWrite(AUD_TRIG_BLUE_PIN, LOW);
          colorTimer = 0;
          Serial.println("PURPLE FOUND!");
        // GREEN
        } else if (gPer > 40) {
          pixels.setPixelColor(0, pixels.Color(0,255,0));
          digitalWrite(AUD_TRIG_GREEN_PIN, LOW);
          colorTimer = 0;
          Serial.println("GREEN FOUND!");
        // UNKNOWN COLOR
        } else {
          pixels.setPixelColor(0, pixels.Color(0,0,0));
          digitalWrite(AUD_TRIG_BLUE_PIN, HIGH);
          digitalWrite(AUD_TRIG_RED_PIN, HIGH);
          digitalWrite(AUD_TRIG_GREEN_PIN, HIGH);
        }
      } else {
        objFound = 1;
      }
    } else {
      objFound = 0;
      pixels.setPixelColor(0, pixels.Color(0,0,0));
      digitalWrite(AUD_TRIG_BLUE_PIN, HIGH);
      digitalWrite(AUD_TRIG_RED_PIN, HIGH);
      digitalWrite(AUD_TRIG_GREEN_PIN, HIGH);
    }
  } else {
    colorTimer += 1;
  }
  pixels.show(); // This sends the updated pixel color to the hardware.

  //**********************************************
  // Get accelerometer data
  //**********************************************
  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  /*Serial.print("X:  "); Serial.print(lis.x); 
  Serial.print("  \tY:  "); Serial.print(lis.y); 
  Serial.print("  \tZ:  "); Serial.print(lis.z);
  Serial.println();*/

  //**********************************************
  // Set LED bar graphs
  //**********************************************
  //switch on the led in the 3'rd row 8'th column
  //and remember that indices start at 0! 
  if (ledDly == 1) {
    if (ledCnt == 8) {
      ledCnt = 0;
      lc.setLed(0,0,1,false);
      lc.setLed(0,1,1,false);
      lc.setLed(0,2,1,false);
      lc.setLed(0,3,1,false);
      lc.setLed(0,4,1,false);
      lc.setLed(0,5,1,false);
      lc.setLed(0,6,1,false);
      lc.setLed(0,7,1,false);
    } else {
      lc.setLed(0,ledCnt,1,true);
      ledCnt = ledCnt + 1;
    }
    ledDly = 0;
  } else {
    ledDly = ledDly + 1;
  }
  
}
