#ifndef BUBBLES_API
#define BUBBLES_API

#include <Arduino.h>

struct BubblesData {
  float interval;
  String name;
  int hue;
};

void getBubblesData(BubblesData& data);

#endif
