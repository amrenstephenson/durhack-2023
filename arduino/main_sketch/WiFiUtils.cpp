#include <WiFi.h>
#include <HTTPClient.h>
#include "WiFiUtils.h"

void connectToWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    const char* ssid = "LinkIT";
    const char* password = "RWvJniP3";
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    for (unsigned int i = 0; WiFi.status() != WL_CONNECTED; i++) {
      delay(500);
      Serial.print(".");
      if (i > 15)
        return;
    }

    Serial.println("");
    Serial.printf("Connected to %s\n", ssid);
    Serial.println("IP address: " + WiFi.localIP().toString());
  }
}

String GET(String url) {
  String resp;

  for (;;) {
    String errMsg = "";
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      Serial.println("Requesting " + url);
      if (http.begin(client, url)) {
        int httpCode = http.GET();
        Serial.println("============== Response code: " + String(httpCode));
        if (httpCode == 200) {
          resp = http.getString();
          http.end();
          break;
        }
        http.end();
      }
      errMsg = "Amren's crappy server is down";
    } else {
      errMsg = "Not connected to LinkIT";
    }
    
    Serial.print("GET failed, ERROR: ");
    Serial.println(errMsg);
    delay(1000);
  }

  return resp;
}
