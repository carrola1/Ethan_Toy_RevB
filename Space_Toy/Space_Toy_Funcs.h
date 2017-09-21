#ifndef _Space_Toy_Funcs_H_
#define _Space_Toy_Funcs_H_

#include <arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_Soundboard.h"

typedef enum
{
  RED     = 0x00,
  BLUE    = 0x01,   
  GREEN   = 0x02,   
  YELLOW  = 0x03,   
  ORANGE  = 0x04,   
  PURPLE  = 0x05,    
  PINK    = 0x06,
  UNKNOWN = 0x07
}
colors_t;

struct RGB_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

colors_t color_identifier(double rPer, double gPer, double bPer);

RGB_t get_rgb(colors_t color);

void play_color_audio(const Adafruit_Soundboard &sfx,colors_t color);

void clear_pixels(const Adafruit_NeoPixel &pixels);

void set_pixels(const Adafruit_NeoPixel &pixels,RGB_t rgb);

#endif