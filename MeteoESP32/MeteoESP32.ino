#include <Arduino.h>

#include "config.h"
#include "data_model.h"
#include "sensors.h"
#include "storage.h"
#include "webui.h"

MeteoData currentData;

unsigned long lastSensorRead = 0;
unsigned long lastSdSave = 0;
unsigned long lastSerialPrint = 0;
unsigned long lastHistoryAdd = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("System start");

  if (!initSensors()) {
    while (1) {
      delay(1000);
    }
  }

  if (!initStorage()) {
    while (1) {
      delay(1000);
    }
  }
  
  Serial.print("Current log file: ");
  Serial.println(getCurrentLogFileName());

  if (!initWebUI(&currentData)) {
    while (1) {
      delay(1000);
    }
  }

  readAllSensors(currentData);
  printDataToSerial(currentData);
  saveToSD(currentData);
  addHistoryPoint(currentData);

  unsigned long now = millis();
  lastSensorRead = now;
  lastSdSave = now;
  lastSerialPrint = now;
  lastHistoryAdd = now;
}

void loop() {
  unsigned long now = millis();

  handleWebClient();

  if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
    lastSensorRead = now;
    readAllSensors(currentData);
  }

  if (now - lastSerialPrint >= SERIAL_PRINT_INTERVAL_MS) {
    lastSerialPrint = now;
    printDataToSerial(currentData);
    Serial.println("------------------------------");
  }

  if (now - lastSdSave >= SD_SAVE_INTERVAL_MS) {
    lastSdSave = now;
    saveToSD(currentData);
  }

  if (now - lastHistoryAdd >= SENSOR_READ_INTERVAL_MS) {
    lastHistoryAdd = now;
    addHistoryPoint(currentData);
  }
}
