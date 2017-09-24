#include "Space_Toy_Funcs.h"
#include <arduino.h>

colors_t color_identifier(double rPer, double gPer, double bPer) {
  // RED (high red, low and equal blue and green)
  if ((rPer > 55) & (abs(gPer-bPer)<=1.0)) {
    return RED;

  // ORANGE (high red, green slightly higher than blue)
  } else if ((rPer > 55) & (gPer > bPer) & (gPer < bPer+5)) {
    return ORANGE;

  // YELLOW
  } else if ((rPer > 40) & (gPer > 35)) {
    return YELLOW;

  // Pink (high red, medium and about equal blue and green)
  } else if ((rPer > 38) & (rPer < 50) & (abs(gPer-bPer)<=1.5)) {
    return PINK;

  // BLUE
  } else if (bPer > 40) {
    return BLUE;

  // PURPLE (medium red and blue)
  } else if ((rPer > 34) & (bPer > 34)) {
    return PURPLE;

  // GREEN
  } else if (gPer > 40) {
    return GREEN;

  // UNKNOWN COLOR
  } else {
    return UNKNOWN;
  }
}

RGB_t get_rgb(colors_t color) {
  switch (color)
  {
    case RED:
      return {255,0,0};
    case ORANGE:
      return {255,60,0};
    case YELLOW:
      return {255,200,0};
    case PINK:
      return {255,10,100};
    case BLUE:
      return {0,50,255};
    case PURPLE:
      return {100,10,255};
    case GREEN:
      return {0,255,0};
    case UNKNOWN:
      return {0,0,0};
  }
}

void play_color_audio(const Adafruit_Soundboard &sfx,colors_t color) {
  switch (color)
  {
    case RED:
      sfx.playTrack("T00     WAV");
    case ORANGE:
      sfx.playTrack("T01     WAV");
    case YELLOW:
      sfx.playTrack("T02     WAV");
    case PINK:
      sfx.playTrack("T03     WAV");
    case BLUE:
      sfx.playTrack("T04     WAV");
    case PURPLE:
      sfx.playTrack("T05     WAV");
    case GREEN:
      sfx.playTrack("T06     WAV");
    case UNKNOWN:
      sfx.playTrack("T05     WAV");
  }
}

void clear_pixels(const Adafruit_NeoPixel &pixels) {
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.setPixelColor(1, pixels.Color(0,0,0));
  pixels.setPixelColor(2, pixels.Color(0,0,0));
  pixels.setPixelColor(3, pixels.Color(0,0,0));
  pixels.setPixelColor(4, pixels.Color(0,0,0));
  pixels.show();
}

void set_pixels(const Adafruit_NeoPixel &pixels,RGB_t rgb) {
  pixels.setPixelColor(0, pixels.Color(rgb.r,rgb.g,rgb.b));
  pixels.setPixelColor(1, pixels.Color(rgb.r,rgb.g,rgb.b));
  pixels.setPixelColor(2, pixels.Color(rgb.r,rgb.g,rgb.b));
  pixels.setPixelColor(3, pixels.Color(rgb.r,rgb.g,rgb.b));
  pixels.setPixelColor(4, pixels.Color(rgb.r,rgb.g,rgb.b));
  pixels.show();
}
