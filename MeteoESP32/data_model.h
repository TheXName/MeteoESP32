#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <Arduino.h>

struct MeteoData {
  float temperature = 0.0;
  float humidity = 0.0;
  float pressure = 0.0;

  double latitude = 0.0;
  double longitude = 0.0;
  double altitude = 0.0;

  String dateStr = "N/A";
  String timeStr = "N/A";
  int satellites = 0;
  String fixStr = "NO";

  bool gpsAltitudeValid = false;
};

#endif
