#include <Wire.h>
#include "DFRobot_HuskylensV2.h"

HuskylensV2 huskylens;

// I2C-Pins explizit definieren
#define SDA_PIN 21
#define SCL_PIN 22

// Pin-Definitionen
int rotPin  = 5;
int blauPin = 2;

// Status: welche LED ist aktuell "aktiv"
enum LedStatus { KEINE, ROT, BLAU };
LedStatus aktuellerStatus = KEINE;

// Für nicht-blockierendes Blinken
unsigned long letzterWechsel = 0;
bool ledAn = false;
const unsigned long blinkIntervall = 500;

void setup() {
  pinMode(rotPin, OUTPUT);
  pinMode(blauPin, OUTPUT);

  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);  // explizite I2C-Pins setzen

  Serial.println("Verbinde mit HuskyLens 2...");
  while (!huskylens.begin(Wire)) {
    Serial.println("HuskyLens 2 nicht gefunden! Verkabelung pruefen...");
    delay(500);
  }

  // Farberkennung aktivieren
  huskylens.switchAlgorithm(ALGORITHM_COLOR_RECOGNITION);

  Serial.println("HuskyLens 2 verbunden! Bereit.");
}

void loop() {
  // 1. Aktuelles Ergebnis der Farberkennung abholen
  huskylens.getResult(ALGORITHM_COLOR_RECOGNITION);

  LedStatus neuerStatus = KEINE;

  if (huskylens.available(ALGORITHM_COLOR_RECOGNITION)) {
    // Prüfen, ob ID2 (Rot) erkannt wurde
    if (huskylens.getCachedResultByID(ALGORITHM_COLOR_RECOGNITION, 2) != NULL) {
      neuerStatus = ROT;
    }
    // Prüfen, ob ID1 (Blau) erkannt wurde
    else if (huskylens.getCachedResultByID(ALGORITHM_COLOR_RECOGNITION, 1) != NULL) {
      neuerStatus = BLAU;
    }
  }

  // 2. Status nur ändern, wenn sich was geändert hat
  if (neuerStatus != aktuellerStatus) {
    aktuellerStatus = neuerStatus;

    if (aktuellerStatus == ROT) {
      digitalWrite(blauPin, LOW);
      Serial.println("-> Rote LED aktiv (ID2 erkannt)");
    } else if (aktuellerStatus == BLAU) {
      digitalWrite(rotPin, LOW);
      Serial.println("-> Blaue LED aktiv (ID1 erkannt)");
    } else {
      digitalWrite(rotPin, LOW);
      digitalWrite(blauPin, LOW);
      Serial.println("-> Nichts erkannt, LEDs aus");
    }
  }

  // 3. Blinken der aktiven LED (nicht blockierend)
  if (aktuellerStatus != KEINE && millis() - letzterWechsel >= blinkIntervall) {
    letzterWechsel = millis();
    ledAn = !ledAn;

    if (aktuellerStatus == ROT) {
      digitalWrite(rotPin, ledAn ? HIGH : LOW);
    } else if (aktuellerStatus == BLAU) {
      digitalWrite(blauPin, ledAn ? HIGH : LOW);
    }
  }

  delay(50); // kleine Pause, damit die HuskyLens nicht überflutet wird
}