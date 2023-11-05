#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Board.h"
#include "StepMotor.h"

// WiFi constants:
#define IP_ADDR "10.247.204.136"
#define PORT "8080"
#define SERVER "http://" IP_ADDR ":" PORT
#define ENDPOINT SERVER "/interval"

// Board PINs:
#define BUTTON1_PIN 13
#define BUTTON2_PIN 15

// Bubble config:
#define RESET_ANGLE 40
#define SQUEEZE_FORCE 80
#define MOTOR_SPEED 3

float bubblesInterval = -1;
bool started = false;

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
  moveAngle(true, RESET_ANGLE + SQUEEZE_FORCE, MOTOR_SPEED);
  moveAngle(false, RESET_ANGLE, MOTOR_SPEED);

  Serial.println("blub");
}

void setup() {
  Serial.begin(115200);

  // setup pins:
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(motorPorts[i], OUTPUT);
  }

  connectToWiFi();
  bubblesInterval = getInterval();

  Serial.printf("bubblesInterval: %f\n", bubblesInterval);
}

void loop() {
  static int prevBubbleTime = 0;
  static bool prevBothDown = false;

  bool btn1Down = digitalRead(BUTTON1_PIN) == LOW;
  bool btn2Down = digitalRead(BUTTON2_PIN) == LOW;
  bool bothDown = btn1Down & btn2Down;

  if (started) {
    if (bubblesInterval > 0) {
      if ((millis() - prevBubbleTime) > bubblesInterval * 1000) {
        makeBubble();
        prevBubbleTime = millis();
      }
    }

    if (!prevBothDown && bothDown) {
      started = false;
      Serial.println("STOPPING!!");
    }
  } else {
    if (!prevBothDown && bothDown) {
      started = true;
      Serial.println("STARTING!!");
    } else if (btn1Down && !btn2Down) {
      moveAngle(true, 1, MOTOR_SPEED);
    } else if (btn2Down && !btn1Down) {
      moveAngle(false, 1, MOTOR_SPEED);
    }
  }

  prevBothDown = bothDown;
}
