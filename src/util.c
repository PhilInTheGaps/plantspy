#include "util.h"

enum { SOIL_WET = 7060, SOIL_DRY = 19136 };

int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

// Based on the arduino map() function:
// https://www.arduino.cc/reference/en/language/functions/math/map/
float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float convert_to_soil_wetness(int value) {
  if (value < 10)
    return 0;
  value = clamp(value, SOIL_WET, SOIL_DRY);
  return map((float)value, (float)SOIL_WET, (float)SOIL_DRY, 100, 0);
}
