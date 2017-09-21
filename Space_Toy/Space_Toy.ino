#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <LedControl.h>
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Space_Toy_Funcs.h"

/* ///// PIN LIST /////
  NEOPIXEL        digital 4 
  SCL             analog 5      
  SDA             analog 4      
  MOSI            digital 11
  SCK             digital 13
  SS              digital 10
  AUD_TRIG_A      analog 0
  AUD_TRIG_B      analog 1
  AUD_TRIG_C      analog 2
  AUD_TRIG_INH    analog 3
  AUD_TRIG_COM    digital 9
  AUD_RST         digital 3
  AUD_TX          digital 12
  AUD_RX          digital 8
  XL_INT          digital 6
  COLOR_LED_EN_N  digital 5
  COLOR_INT       analog 7
*/

//**********************************************
// Serial Port Print Enables
//**********************************************
#define SER_COL_ENB 0
#define SER_XL_ENB 0

//**********************************************
// Initialize color sensor
//**********************************************
#define COLOR_LED_PIN 5
#define COLOR_INT_PIN A7
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_1X);
colors_t current_color;
colors_t last_color;

//**********************************************
// Initialize neopixel
//**********************************************
#define NEO_PIN   4
#define NUMPIXELS 5
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

//**********************************************
// Initialize Audio FX Interface
//**********************************************
#define AUD_TRIG_A_PIN A0
#define AUD_TRIG_B_PIN A1
#define AUD_TRIG_C_PIN A2
#define AUD_TRIG_INH_PIN A3
#define AUD_TRIG_COM_PIN 9
#define AUD_RX_PIN 8
#define AUD_TX_PIN 12
#define AUD_RST_PIN 3
SoftwareSerial ss = SoftwareSerial(AUD_TX_PIN, AUD_RX_PIN);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, AUD_RST_PIN);

//**********************************************
// Initialize accelerometer
//**********************************************
#define XL_INT_PIN 6
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
uint16_t xlTimer;
#define XL_TO 35
#define XL_WAKE_THRESH 2000
#define XL_INIT_THRESH 9900
#define XL_BUILD_THRESH 9500
int16_t last_x, last_y, last_z;
 
//**********************************************
// Initialize LED Driver
//**********************************************
#define LED_MOSI_PIN 11
#define LED_SCK_PIN  13
#define LED_SS_PIN   10
LedControl lc=LedControl(LED_MOSI_PIN,LED_SCK_PIN,LED_SS_PIN,1);
uint8_t ledCnt;
#define LED_DLY 50  // delay between LED lit and next time we look at XL (ms)

//**********************************************
// State Machine
//**********************************************
typedef enum
{
  SLEEP     = 0x00,
  DETECT    = 0x01,   
  COUNTDOWN = 0x02,   
  BLASTOFF  = 0x03   
}
state_t;
state_t state,nextState;
#define SLEEP_TO 800  // ~2 minutes
uint16_t sleepTimer;
bool ledGraphEnb;

void setup(void) {

  delay(500); // wait to allow power to stabilize
  
  //**********************************************
  // Setup serial port
  //**********************************************
  Serial.begin(9600);

  //**********************************************
  // Setup color sensor
  //**********************************************
  pinMode(COLOR_INT_PIN, INPUT);
  pinMode(COLOR_LED_PIN, OUTPUT);
  digitalWrite(COLOR_LED_PIN, HIGH);
  tcs.begin();
  current_color = UNKNOWN;
  last_color    = UNKNOWN;

  //**********************************************
  // Setup neopixel
  //**********************************************
  pixels.begin(); // This initializes the NeoPixel library.
  clear_pixels(pixels);

  //**********************************************
  // Setup Audio FX trigger interface
  //**********************************************
  pinMode(AUD_TRIG_A_PIN, OUTPUT);
  pinMode(AUD_TRIG_B_PIN, OUTPUT);
  pinMode(AUD_TRIG_C_PIN, OUTPUT);
  pinMode(AUD_TRIG_INH_PIN, OUTPUT);
  pinMode(AUD_TRIG_COM_PIN, OUTPUT);
  digitalWrite(AUD_TRIG_A_PIN, LOW); 
  digitalWrite(AUD_TRIG_B_PIN, LOW);
  digitalWrite(AUD_TRIG_C_PIN, LOW);
  digitalWrite(AUD_TRIG_INH_PIN, HIGH);
  digitalWrite(AUD_TRIG_COM_PIN, LOW);
  ss.begin(9600);
  
  //**********************************************
  // Setup accelerometer
  //**********************************************
  pinMode(XL_INT_PIN, INPUT);
  lis.begin(0x18);
  lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G
  xlTimer = 0;
  last_x = 0;
  last_y = 0;
  last_z = 0;

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

  //**********************************************
  // Setup state machine
  //**********************************************
  state = DETECT;
  nextState = DETECT;
  ledGraphEnb = false;
}

void loop(void) {

  //**********************************************
  // Get accelerometer data
  //**********************************************
  lis.read();      // get X Y and Z data at once
  int16_t current_x = lis.x;
  int16_t current_y = lis.y;
  int16_t current_z = lis.z;

  if (SER_XL_ENB == 1) {
    Serial.print("X:  "); Serial.print(current_x); 
    Serial.print("  \tY:  "); Serial.print(current_y); 
    Serial.print("  \tZ:  "); Serial.print(current_z);
    Serial.println();
  }

  switch (state)
  {
    // Turn everything off except accelerometer to conserve power
    case SLEEP:
      Serial.println("SLEEP");
      if ((abs(current_x-last_x) > XL_WAKE_THRESH) || 
          (abs(current_y-last_y) > XL_WAKE_THRESH) ||
          (abs(current_z-last_z) > XL_WAKE_THRESH)) {
        digitalWrite(COLOR_LED_PIN, HIGH);
        lc.shutdown(0,false);
        nextState = DETECT;
      }
      break;

    case DETECT:
      Serial.println("DETECT");
      if (sleepTimer == SLEEP_TO) {
        digitalWrite(COLOR_LED_PIN, LOW);
        lc.shutdown(0,true);
        nextState = SLEEP;
      } else {
        if ((abs(current_x) > XL_INIT_THRESH) || 
            (abs(current_y) > XL_INIT_THRESH) || 
            (abs(current_z) > XL_INIT_THRESH)) {
          xlTimer    = 0;
          sleepTimer = 0;
          digitalWrite(COLOR_LED_PIN, LOW);
          sfx.playTrack("T07     WAV");
          ledCnt  = 0;
          nextState   = COUNTDOWN;
        } else if (current_color != UNKNOWN) {
          sleepTimer = 0;
        } else {
          sleepTimer += 1;
        }
      }
      break;

    case COUNTDOWN:
      Serial.println("COUNTDOWN");
      if (xlTimer == XL_TO) {
        nextState = DETECT;
        sfx.stop();
        lc.clearDisplay(0);
        digitalWrite(COLOR_LED_PIN, HIGH);
        ledCnt = 0;
        ledGraphEnb = false;
      } else {
        if ((abs(current_x) > XL_BUILD_THRESH) || 
            (abs(current_y) > XL_BUILD_THRESH) || 
            (abs(current_z) > XL_BUILD_THRESH)) {
          ledGraphEnb = true;
          xlTimer = 0;
          if (ledCnt == 30) {
            sfx.stop();
            sfx.playTrack("T08     WAV");
            nextState = BLASTOFF;
          }
        } else {
          ledGraphEnb = false;
          xlTimer += 1;
        }
        delay(LED_DLY); // slow it down
      }
      break;

    case BLASTOFF:
      Serial.println("BLASTOFF");
      RGB_t pixels_rgb = get_rgb(RED);
      set_pixels(pixels,pixels_rgb);
      delay(2500);
      clear_pixels(pixels);
      lc.clearDisplay(0);
      ledCnt = 0;
      ledGraphEnb = false;
      digitalWrite(COLOR_LED_PIN, HIGH);
      nextState = DETECT;
      break;
  }
  state = nextState;
  last_x = current_x;
  last_y = current_y;
  last_z = current_z;

  //**********************************************
  // Update LED bar graphs
  //**********************************************
  if (ledGraphEnb == true) {
    if (ledCnt == 30) {
      // Do nothing
    } else {
      if (ledCnt < 8) {
        lc.setLed(0,ledCnt,0,true);
        lc.setLed(0,ledCnt,4,true);
      } else if (ledCnt < 16) {
        lc.setLed(0,ledCnt-8,1,true);
        lc.setLed(0,ledCnt-8,5,true);
      } else if (ledCnt < 24) {
        lc.setLed(0,ledCnt-16,2,true);
        lc.setLed(0,ledCnt-16,6,true);
      } else {
        lc.setLed(0,ledCnt-24,3,true);
        lc.setLed(0,ledCnt-24,7,true);
      }
      ledCnt = ledCnt + 1;
    }
  }

  if (state == DETECT) {   // only check on color sensor if LED graph not activated
    
    //********************************************** 
    // Get color sensor data
    //**********************************************
    uint16_t r, g, b, c;
    double rPer,gPer,bPer,rgbTot;
    
    tcs.getRawData(&r, &g, &b, &c);
    
    // Calculate individual RGB percentages of total
    rgbTot = double(r + b + g + 1);
    rPer = r/rgbTot*100.0;
    gPer = g/rgbTot*100.0;
    bPer = b/rgbTot*100.0;
  
    if (SER_COL_ENB == 1) {
      Serial.print("R: "); Serial.print(rPer, DEC); Serial.print(" ");
      Serial.print("G: "); Serial.print(gPer, DEC); Serial.print(" ");
      Serial.print("B: "); Serial.print(bPer, DEC); Serial.print(" ");
      Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
      Serial.println(" ");
    }
    
    //**********************************************
    // Update Neopixel color and trigger audio
    //**********************************************
    if (c > 1500) {
      current_color = color_identifier(rPer,gPer,bPer);
      if (current_color == UNKNOWN) {
        if (last_color != UNKNOWN) {
          clear_pixels(pixels);
        }
      } else {
        RGB_t pixels_rgb = get_rgb(current_color);
        digitalWrite(COLOR_LED_PIN, LOW);
        set_pixels(pixels,pixels_rgb);
        play_color_audio(sfx,current_color);
        delay(4500);
        clear_pixels(pixels);
        digitalWrite(COLOR_LED_PIN, HIGH);
        last_color = current_color;
      }
    } else {
      current_color = UNKNOWN;
    }
  } 

  


  /*bool ledGraphEnb,ledGraphRes;
  if ((abs(lis.x) > XL_THRESH) || (abs(lis.y) > XL_THRESH) || (abs(lis.z) > XL_THRESH)) {
    ledGraphEnb = true;
    ledGraphRes = false;
    xlTimer = 0;
  } else if (xlTimer == XL_TO) {
    ledGraphRes = true;
    ledGraphEnb = false;
    xlTimer = xlTimer + 1;
  } else if (xlTimer == XL_TO+1) {
    ledGraphRes = false;
    ledGraphEnb = false;
  } else {
    ledGraphEnb = false;
    ledGraphEnb = false;
    xlTimer = xlTimer + 1;
  }*/

  /*if (xlTimer < XL_TO) {
    delay(LED_DLY);  // slow it down
  }*/
  
}
