#include <Arduino.h>
#include "Board.h"
#include "StepMotor.h"
#include "WiFiUtils.h"
#include "BubblesAPI.h"

// Board PINs:
#define BUTTON1_PIN 13
#define BUTTON2_PIN 15

// Bubble config:
#define RESET_ANGLE 40
#define SQUEEZE_FORCE 80
#define MOTOR_SPEED 3

// Other:
#define API_POLL_INTERVAL_MS 1000

float bubblesInterval = -1;
bool started = false;
BubblesData bubblesData;

void setup() {
  Serial.begin(115200);

  // setup pins:
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(motorPorts[i], OUTPUT);
  }

  connectToWiFi();
}

void makeBubble() {
  moveAngle(true, RESET_ANGLE + SQUEEZE_FORCE, MOTOR_SPEED);
  moveAngle(false, RESET_ANGLE, MOTOR_SPEED);

  Serial.println("blub");
}

void loop() {
  static int prevBubbleTime = 0;
  static int prevPollAPITime = 0;
  static bool prevBothDown = false;

  bool btn1Down = digitalRead(BUTTON1_PIN) == LOW;
  bool btn2Down = digitalRead(BUTTON2_PIN) == LOW;
  bool bothDown = btn1Down & btn2Down;

  if (started) {
    // Poll API every API_POLL_INTERVAL_MS ms:
    if ((millis() - prevPollAPITime) > API_POLL_INTERVAL_MS) {
      getBubblesData(bubblesData);
      prevPollAPITime = millis();
    }

    // Make bubbles at intervals:
    if (bubblesData.interval >= 0) {
      if ((millis() - prevBubbleTime) > bubblesData.interval * 1000) {
        makeBubble();
        prevBubbleTime = millis();
      }
    }

    // both buttons held -> STOP:
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
