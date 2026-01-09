#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HCSR04.h>
// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
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
// For å unngå å tegne skjerm unødvendig ofte
double lastShownDist = -999;
bool lastShownDetected = false;
bool lastShownDisabled = false;
void showOnDisplay(double distCm, bool detected, bool disabled) {
  // Oppdater kun hvis noe har endret seg “nok”
  if (!disabled) {
    if (fabs(distCm - lastShownDist) < 0.2 &&
        detected == lastShownDetected &&
        disabled == lastShownDisabled) {
      return;
    }
  } else {
    if (disabled == lastShownDisabled) return;
  }
  lastShownDist = distCm;
  lastShownDetected = detected;
  lastShownDisabled = disabled;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("HC-SR04"));
  if (disabled) {
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println(F("TOG!!"));
    display.display();
    return;
  }
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print(F("Avstand: "));
  display.print(distCm, 1);
  display.println(F(" cm"));
  display.setCursor(0, 34);
  display.print(F("Status: "));
  display.println(detected ? F("Objekt!") : F("Ingen"));
  display.display();
}
void setup() {
  Serial.begin(9600);
  pinMode(ID_PIN, INPUT_PULLUP);
  // Start OLED
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 init feilet"));
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Starter..."));
  display.display();
  // Start HC-SR04
  HCSR04.begin(TRIG_PIN, echoPins, ECHO_COUNT);
  Serial.println(F("HC-SR04 start"));
  // Vis init-status
  showOnDisplay(0.0, false, false);
}
void loop() {
  unsigned long now = millis();
  if (now - lastReadMs < READ_INTERVAL_MS) return;
  lastReadMs = now;
  // Disable: A0 LOW -> ikke mål / ikke print
  if (digitalRead(ID_PIN) == LOW) {
    underSinceMs = 0;
    objectDetected = false;
    showOnDisplay(0.0, false, true);
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
  // Oppdater display
  showOnDisplay(dist, objectDetected, false);
}
