#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include "data_model.h"

bool initWebUI(MeteoData* dataPtr);
void handleWebClient();
void addHistoryPoint(const MeteoData& data);

#endif
