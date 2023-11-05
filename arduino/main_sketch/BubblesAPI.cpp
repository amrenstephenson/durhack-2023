#include "BubblesAPI.h"
#include "WiFiUtils.h"

// WiFi constants:
#define IP_ADDR "10.247.204.136"
#define PORT "8080"
#define SERVER "http://" IP_ADDR ":" PORT
#define ENDPOINT SERVER "/api/bubbles"

void getBubblesData(BubblesData& data) {
  // query endpoint:
  // String csvString = GET(ENDPOINT);

  String csvString = "2.0,test name";

  // parse CSV:
  const char* delims = ",";
  char* s = strdup(csvString.c_str());
  const char* part = strtok(s, delims);
  unsigned int i = 0;
  while (part) {
    switch (i++) {
      case 0:
        data.interval = atof(part);
        Serial.printf("Interval: %f\n", data.interval);
        break;
      case 1:
        data.name = part;
        Serial.printf("Name: %s\n", data.name.c_str());
        break;
      default:
        break;
    }
    part = strtok(NULL, delims);
  }
  free(s);
}
