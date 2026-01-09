#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HCSR04.h>
#include <Arduino.h>

const uint8_t TRIG_PIN = 9;
const uint8_t ECHO_PIN = 10;

const uint8_t ID_PIN = A0;

const uint8_t LED_RED_PIN = 7;
const uint8_t LED_GREEN_PIN = 8;

const double DETECT_THRESHOLD_CM = 20.0;
const uint16_t READ_INTERVAL_MS = 100;
const uint16_t DETECT_HOLD_MS = 200;

const uint8_t ECHO_COUNT = 1;
uint8_t echoPins[ECHO_COUNT] = { ECHO_PIN };

unsigned long lastReadMs = 0;
unsigned long underSinceMs = 0;
bool objectDetected = false;

void setLeds(bool led1, bool led2);

void setup() {
  Serial.begin(9600);

  pinMode(ID_PIN, INPUT_PULLUP);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);

  HCSR04.begin(TRIG_PIN, echoPins, ECHO_COUNT);

  setLeds(false, false);
}

void loop() {
  unsigned long now = millis();
  if (now - lastReadMs < READ_INTERVAL_MS) return;
  lastReadMs = now;

  bool enabled = (digitalRead(ID_PIN) == HIGH);

  if (!enabled) {
    underSinceMs = 0;
    if (objectDetected) {
      objectDetected = false;
      Serial.println(F("Objekt borte."));
    }
    setLeds(false, false);
    return;
  }

  double* dists = HCSR04.measureDistanceCm();
  double dist = dists[0];

  if (dist <= 0) {
    setLeds(true, objectDetected);
    return;
  }

  if (dist <= DETECT_THRESHOLD_CM) {
    if (!objectDetected) {
      if (underSinceMs == 0) underSinceMs = now;
      if (now - underSinceMs >= DETECT_HOLD_MS) {
        objectDetected = true;
        Serial.println(F("Objekt!"));
      }
    }
  } else {
    underSinceMs = 0;
    if (objectDetected) {
      objectDetected = false;
      Serial.println(F("Objekt borte."));
    }
  }

  setLeds(true, objectDetected);
}

void setLeds(bool enabled, bool detected) {
  digitalWrite(LED_GREEN_PIN, HIGH);

  if (!enabled) {
    digitalWrite(LED_RED_PIN, LOW);
    return;
  }

  digitalWrite(LED_RED_PIN, detected ? HIGH : LOW);
  if (detected) digitalWrite(LED_GREEN_PIN, LOW);
}

// Tormund = Soulless
// Lars = Sleepmaster