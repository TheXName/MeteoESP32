#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "data_model.h"

bool initSensors();
void readBME(MeteoData& data);
void readGPS(MeteoData& data);
void readAllSensors(MeteoData& data);
void printDataToSerial(const MeteoData& data);

#endif
