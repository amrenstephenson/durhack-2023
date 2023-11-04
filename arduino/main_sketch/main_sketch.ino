#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include "Board.h"

// WiFi constants:
#define IP_ADDR "10.247.204.136"
#define PORT "8080"
#define SERVER "http://" IP_ADDR ":" PORT
#define ENDPOINT SERVER "/interval"

// Board PINs:
#define SERVO_PIN 15
#define BUTTON_PIN 13

// Bubble config:
#define INIT_SERVO_ANGLE 90
#define FINAL_SERVO_ANGLE 0
#define VALVE_OPEN_DURATION 100 // ms

Servo servo;
float bubblesInterval = -1;
bool killed = false;

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

float getInterval() {
  String intervalString = GET(ENDPOINT);
  return intervalString.toFloat();
}

void makeBubble() {
  servo.write(FINAL_SERVO_ANGLE);
  delay(VALVE_OPEN_DURATION);
  servo.write(INIT_SERVO_ANGLE);

  Serial.println("blub");
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT);

  connectToWiFi();
  bubblesInterval = getInterval();
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2500);
  servo.write(INIT_SERVO_ANGLE);

  Serial.printf("bubblesInterval: %f\n", bubblesInterval);
}

void loop() {
  if (!killed && digitalRead(BUTTON_PIN) == LOW) {
    killed = true;
    Serial.println("KILLING EVERYING!!");
  }

  if (!killed && bubblesInterval > 0) {
    delay(1000 * bubblesInterval);
    makeBubble();
  }
}
