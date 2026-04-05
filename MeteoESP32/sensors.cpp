#include "sensors.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS++.h>
#include "config.h"

static Adafruit_BME280 bme;
static TinyGPSPlus gps;
static HardwareSerial gpsSerial(2);

bool initSensors() {
  Wire.begin();

  if (!bme.begin(0x76)) {
    Serial.println("ERROR: BME280 not found!");
    return false;
  }
  Serial.println("BME280 OK");

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS OK");

  return true;
}

void readBME(MeteoData& data) {
  data.temperature = bme.readTemperature();
  data.humidity = bme.readHumidity();
  data.pressure = bme.readPressure() / 100.0f;
}

void readGPS(MeteoData& data) {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  data.latitude = 0.0;
  data.longitude = 0.0;
  data.altitude = 0.0;
  data.dateStr = "N/A";
  data.timeStr = "N/A";
  data.satellites = 0;
  data.fixStr = "NO";
  data.gpsAltitudeValid = false;

  if (gps.location.isValid()) {
    data.latitude = gps.location.lat();
    data.longitude = gps.location.lng();
    data.fixStr = "YES";
  }

  if (gps.altitude.isValid()) {
    data.altitude = gps.altitude.meters();
    data.gpsAltitudeValid = true;
  }

  if (gps.date.isValid()) {
    data.dateStr = String(gps.date.day()) + "." +
                   String(gps.date.month()) + "." +
                   String(gps.date.year());
  }

  if (gps.time.isValid()) {
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d",
            gps.time.hour(),
            gps.time.minute(),
            gps.time.second());
    data.timeStr = String(buf);
  }

  if (gps.satellites.isValid()) {
    data.satellites = gps.satellites.value();
  }
}

void readAllSensors(MeteoData& data) {
  readGPS(data);
  readBME(data);
}

void printDataToSerial(const MeteoData& data) {
  Serial.println("===== BME280 =====");
  Serial.print("Temperature = ");
  Serial.print(data.temperature, 2);
  Serial.println(" °C");

  Serial.print("Humidity = ");
  Serial.print(data.humidity, 2);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(data.pressure, 2);
  Serial.println(" hPa");

  Serial.println("===== GPS =====");
  Serial.print("Fix: ");
  Serial.println(data.fixStr);

  Serial.print("Satellites: ");
  Serial.println(data.satellites);

  Serial.print("Latitude: ");
  if (data.fixStr == "YES") Serial.println(data.latitude, 6);
  else Serial.println("N/A");

  Serial.print("Longitude: ");
  if (data.fixStr == "YES") Serial.println(data.longitude, 6);
  else Serial.println("N/A");

  Serial.print("Altitude (m): ");
  if (data.gpsAltitudeValid) Serial.println(data.altitude, 2);
  else Serial.println("N/A");

  Serial.print("Date: ");
  Serial.println(data.dateStr);

  Serial.print("Time (UTC): ");
  Serial.println(data.timeStr);
}
