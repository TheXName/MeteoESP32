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

static bool isCsvFileName(const String& name) {
  return name.endsWith(".csv");
}

String getCsvFileListJson() {
  String json = "[";
  bool first = true;

  File root = SD.open("/");
  if (!root) {
    return "[]";
  }

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }

    if (!entry.isDirectory()) {
      String name = String(entry.name());

      if (!name.startsWith("/")) {
        name = "/" + name;
      }

      if (isCsvFileName(name)) {
        if (!first) {
          json += ",";
        }

        json += "{";
        json += "\"name\":\"" + name + "\",";
        json += "\"is_current\":";
        json += (name == currentLogFileName) ? "true" : "false";
        json += "}";

        first = false;
      }
    }

    entry.close();
  }

  root.close();
  json += "]";
  return json;
}

bool deleteCsvFile(const String& fileName) {
  if (fileName == "") {
    return false;
  }

  String normalized = fileName;
  if (!normalized.startsWith("/")) {
    normalized = "/" + normalized;
  }

  if (normalized == currentLogFileName) {
    Serial.println("ERROR: Attempt to delete current log file");
    return false;
  }

  if (!normalized.endsWith(".csv")) {
    Serial.println("ERROR: Only CSV files can be deleted");
    return false;
  }

  if (!SD.exists(normalized)) {
    Serial.println("ERROR: File does not exist");
    return false;
  }

  bool ok = SD.remove(normalized);

  if (ok) {
    Serial.print("Deleted file: ");
    Serial.println(normalized);
  } else {
    Serial.print("ERROR: Failed to delete file: ");
    Serial.println(normalized);
  }

  return ok;
}
