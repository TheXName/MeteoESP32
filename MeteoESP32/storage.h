#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "data_model.h"

bool initStorage();
void saveToSD(const MeteoData& data);
String getCurrentLogFileName();

String getCsvFileListJson();
bool deleteCsvFile(const String& fileName);

#endif
