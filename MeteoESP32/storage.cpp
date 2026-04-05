#include "storage.h"

#include <SPI.h>
#include <SD.h>
#include "config.h"

static String currentLogFileName = "";

static String generateNewFileName() {
  int index = 1;

  while (true) {
    String fileName = "/data_" + String(index) + ".csv";

    if (!SD.exists(fileName)) {
      return fileName;
    }

    index++;
  }
}

String getCurrentLogFileName() {
  return currentLogFileName;
}

bool initStorage() {
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR: SD card initialization failed!");
    return false;
  }
  Serial.println("SD OK");

  currentLogFileName = generateNewFileName();

  File file = SD.open(currentLogFileName, FILE_WRITE);
  if (!file) {
    Serial.println("ERROR: Cannot create log file");
    return false;
  }

  file.println("temperature_c,humidity_percent,pressure_hpa,latitude,longitude,altitude_m,date,time_utc,satellites,fix");
  file.close();

  Serial.print("Log file created: ");
  Serial.println(currentLogFileName);

  return true;
}

void saveToSD(const MeteoData& data) {
  if (currentLogFileName == "") {
    Serial.println("ERROR: No log file name");
    return;
  }

  File file = SD.open(currentLogFileName, FILE_APPEND);

  if (!file) {
    Serial.println("SD write error");
    return;
  }

  file.print(data.temperature, 2);
  file.print(",");

  file.print(data.humidity, 2);
  file.print(",");

  file.print(data.pressure, 2);
  file.print(",");

  if (data.fixStr == "YES") file.print(data.latitude, 6);
  else file.print("N/A");
  file.print(",");

  if (data.fixStr == "YES") file.print(data.longitude, 6);
  else file.print("N/A");
  file.print(",");

  if (data.gpsAltitudeValid) file.print(data.altitude, 2);
  else file.print("N/A");
  file.print(",");

  file.print(data.dateStr);
  file.print(",");

  file.print(data.timeStr);
  file.print(",");

  file.print(data.satellites);
  file.print(",");

  file.println(data.fixStr);

  file.close();
  Serial.println("Saved to SD");
}
