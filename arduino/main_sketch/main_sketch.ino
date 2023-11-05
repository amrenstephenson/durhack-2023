#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Freenove_WS2812_Lib_for_ESP32.h>
#include "Board.h"
#include "StepMotor.h"
#include "WiFiUtils.h"
#include "BubblesAPI.h"

// Board PINs:
#define BUTTON1_PIN 13
#define BUTTON2_PIN 15
#define SDA_PIN 33
#define SCL_PIN 32
#define LEDS_COUNT 8
#define LEDS_PIN 2
#define CHANNEL 0

// Bubble config:
#define RESET_ANGLE 40
#define SQUEEZE_FORCE 80
#define MOTOR_SPEED 3

// Other:
#define API_POLL_INTERVAL_MS 1000

float bubblesInterval = -1;
bool started = false;
BubblesData bubblesData;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Freenove_ESP32_WS2812 ledStrip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void setup() {
  Serial.begin(115200);

  // setup pins:
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(motorPorts[i], OUTPUT);
  }
  
  // setup LCD:
  Wire.begin(SDA_PIN, SCL_PIN);   // attach the IIC pin
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcdUpdate();

  // setup LEDs:
  ledStrip.begin();
  ledStrip.setBrightness(10);
  ledOff();

  // connect to wifi:
  connectToWiFi();
}

bool i2CAddrTest(uint8_t addr) {
  Wire.begin();
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}

void makeBubble() {
  ledOn();
  moveAngle(true, RESET_ANGLE + SQUEEZE_FORCE, MOTOR_SPEED);
  moveAngle(false, RESET_ANGLE, MOTOR_SPEED);
  ledOff();

  Serial.println("blub");
}

void lcdUpdate() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(String("Status: ") + (started ? "On" : "Off"));
  if (started) {
    lcd.setCursor(0,1);
    lcd.print(bubblesData.name);
  }
}

void ledOn() {
  for (int i = 0; i < LEDS_COUNT; i++) {
    ledStrip.setLedColorData(i, ledStrip.Wheel(bubblesData.hue));
  }
  ledStrip.show();
}

void ledOff() {
  for (int i = 0; i < LEDS_COUNT; i++) {
    ledStrip.setLedColorData(i, 0, 0, 0);
  }
  ledStrip.show();
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

      // update new data on LCD:
      lcdUpdate();
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

      lcdUpdate();
    }
  } else {
    if (!prevBothDown && bothDown) {
      started = true;
      Serial.println("STARTING!!");

      lcdUpdate();
    } else if (btn1Down && !btn2Down) {
      moveAngle(true, 1, MOTOR_SPEED);
    } else if (btn2Down && !btn1Down) {
      moveAngle(false, 1, MOTOR_SPEED);
    }
  }

  prevBothDown = bothDown;
}
