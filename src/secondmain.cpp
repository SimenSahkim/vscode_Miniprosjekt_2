#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HCSR04.h>
// ---------- OLED ----------
// ---------- HC-SR04 ----------
const uint8_t TRIG_PIN = 9;
const uint8_t ECHO_PIN = 10;
// Enable/disable inngang (pull-up): HIGH normalt, LOW = disable
const uint8_t ID_PIN   = A0;
// Parametre
const double   DETECT_THRESHOLD_CM = 20.0;
const uint16_t READ_INTERVAL_MS    = 100;
const uint16_t DETECT_HOLD_MS      = 200;
// HCSR04 init (biblioteket vil ha array av echo-pinner)
const uint8_t ECHO_COUNT = 1;
uint8_t echoPins[ECHO_COUNT] = { ECHO_PIN };
// Tilstand
unsigned long lastReadMs = 0;
unsigned long underSinceMs = 0;
bool objectDetected = false;

void setup() {
  Serial.begin(9600);
  pinMode(ID_PIN, INPUT_PULLUP);
  // Start HC-SR04
  HCSR04.begin(TRIG_PIN, echoPins, ECHO_COUNT);
  Serial.println(F("HC-SR04 start"));
  // Vis init-status
}
void loop() {
  unsigned long now = millis();
  if (now - lastReadMs < READ_INTERVAL_MS) return;
  lastReadMs = now;
  // Disable: A0 LOW -> ikke mål / ikke print
  if (digitalRead(ID_PIN) == LOW) {
    underSinceMs = 0;
    objectDetected = false;
    return;
  }
  // Mål avstand
  double* dists = HCSR04.measureDistanceCm();
  double dist = dists[0];
  // Ugyldig/timeout
  if (dist <= 0) return;
  // Deteksjon kun basert på avstand + hold-tid
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

}