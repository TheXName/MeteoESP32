#include "webui.h"

#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <math.h>

#include "config.h"
#include "storage.h"

static MeteoData* gData = nullptr;
static WebServer server(80);

const int HISTORY_SIZE = 50;

static float pressureHistory[HISTORY_SIZE];
static float altitudeHistory[HISTORY_SIZE];

static int historyCount = 0;
static int totalPointIndex = 0;

void addHistoryPoint(const MeteoData& data) {
  if (historyCount < HISTORY_SIZE) {
    pressureHistory[historyCount] = data.pressure;
    altitudeHistory[historyCount] = data.gpsAltitudeValid ? data.altitude : NAN;
    historyCount++;
  } else {
    for (int i = 1; i < HISTORY_SIZE; i++) {
      pressureHistory[i - 1] = pressureHistory[i];
      altitudeHistory[i - 1] = altitudeHistory[i];
    }

    pressureHistory[HISTORY_SIZE - 1] = data.pressure;
    altitudeHistory[HISTORY_SIZE - 1] = data.gpsAltitudeValid ? data.altitude : NAN;
  }

  totalPointIndex++;
}

static String buildHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Meteo ESP32</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      background: #f5f5f5;
      color: #222;
    }

    h1 {
      font-size: 24px;
      margin-bottom: 16px;
    }

    .card {
      background: white;
      border-radius: 12px;
      padding: 16px;
      margin-bottom: 16px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.08);
    }

    .value {
      font-size: 20px;
      margin: 8px 0;
    }

    .small {
      color: #666;
      font-size: 14px;
    }

    .ok {
      color: green;
      font-weight: bold;
    }

    .bad {
      color: red;
      font-weight: bold;
    }

    .btn {
      display: inline-block;
      padding: 10px 14px;
      background: #0077cc;
      color: white;
      text-decoration: none;
      border-radius: 8px;
      font-size: 16px;
      margin-top: 10px;
    }

    canvas {
      width: 100%;
      max-width: 800px;
      height: 260px;
      border: 1px solid #ddd;
      border-radius: 8px;
      background: #fff;
      display: block;
      margin-top: 12px;
    }
  </style>
</head>
<body>
  <h1>Meteo ESP32</h1>

  <div class="card">
    <div class="value">Temperature: <span id="temperature">--</span> °C</div>
    <div class="value">Humidity: <span id="humidity">--</span> %</div>
    <div class="value">Pressure: <span id="pressure">--</span> hPa</div>
  </div>

  <div class="card">
    <div class="value">Fix: <span id="fix">--</span></div>
    <div class="value">Satellites: <span id="satellites">--</span></div>
    <div class="value">Latitude: <span id="latitude">--</span></div>
    <div class="value">Longitude: <span id="longitude">--</span></div>
    <div class="value">Altitude: <span id="altitude">--</span></div>
    <div class="value">Date: <span id="date">--</span></div>
    <div class="value">Time (UTC): <span id="time">--</span></div>
  </div>

  <div class="card">
    <div class="value">Current CSV log file:</div>
    <div class="small" id="logFileName">loading...</div>
    <a class="btn" href="/download">Download CSV</a>
  </div>

  <div class="card">
    <div class="value">Pressure history</div>
    <canvas id="pressureChart" width="800" height="260"></canvas>
  </div>

  <div class="card">
    <div class="value">Altitude history</div>
    <canvas id="altitudeChart" width="800" height="260"></canvas>
  </div>

  <div class="card small">
    Data updates automatically every 2 seconds.
  </div>

  <script>
    function drawChart(canvasId, values, title, unit) {
      const canvas = document.getElementById(canvasId);
      const ctx = canvas.getContext('2d');

      ctx.clearRect(0, 0, canvas.width, canvas.height);

      if (!values || values.length === 0) {
        ctx.font = '16px Arial';
        ctx.fillText('No data', 20, 40);
        return;
      }

      const validValues = values.filter(v => v !== null && !Number.isNaN(v));
      if (validValues.length === 0) {
        ctx.font = '16px Arial';
        ctx.fillText('No valid data', 20, 40);
        return;
      }

      const padding = 40;
      const w = canvas.width;
      const h = canvas.height;

      let minVal = Math.min(...validValues);
      let maxVal = Math.max(...validValues);

      if (minVal === maxVal) {
        minVal -= 1;
        maxVal += 1;
      }

      ctx.strokeStyle = '#cccccc';
      ctx.lineWidth = 1;

      ctx.beginPath();
      ctx.moveTo(padding, padding);
      ctx.lineTo(padding, h - padding);
      ctx.lineTo(w - padding, h - padding);
      ctx.stroke();

      ctx.fillStyle = '#222';
      ctx.font = '14px Arial';
      ctx.fillText(title, padding, 20);
      ctx.fillText(maxVal.toFixed(2) + ' ' + unit, 5, padding + 5);
      ctx.fillText(minVal.toFixed(2) + ' ' + unit, 5, h - padding);

      const stepX = values.length > 1 ? (w - 2 * padding) / (values.length - 1) : 0;

      ctx.strokeStyle = '#0077cc';
      ctx.lineWidth = 2;
      ctx.beginPath();

      let started = false;

      for (let i = 0; i < values.length; i++) {
        const v = values[i];
        if (v === null || Number.isNaN(v)) continue;

        const x = padding + i * stepX;
        const y = h - padding - ((v - minVal) / (maxVal - minVal)) * (h - 2 * padding);

        if (!started) {
          ctx.moveTo(x, y);
          started = true;
        } else {
          ctx.lineTo(x, y);
        }
      }

      ctx.stroke();
    }

    async function updateData() {
      try {
        const response = await fetch('/data');
        const data = await response.json();

        document.getElementById('temperature').textContent = data.temperature.toFixed(2);
        document.getElementById('humidity').textContent = data.humidity.toFixed(2);
        document.getElementById('pressure').textContent = data.pressure.toFixed(2);

        const fixElem = document.getElementById('fix');
        fixElem.textContent = data.fix;
        fixElem.className = (data.fix === 'YES') ? 'ok' : 'bad';

        document.getElementById('satellites').textContent = data.satellites;
        document.getElementById('latitude').textContent = (data.latitude !== null) ? data.latitude.toFixed(6) : 'N/A';
        document.getElementById('longitude').textContent = (data.longitude !== null) ? data.longitude.toFixed(6) : 'N/A';
        document.getElementById('altitude').textContent = (data.altitude !== null) ? data.altitude.toFixed(2) + ' m' : 'N/A';
        document.getElementById('date').textContent = data.date;
        document.getElementById('time').textContent = data.time_utc;
        document.getElementById('logFileName').textContent = data.log_file;

        drawChart('pressureChart', data.pressure_history, 'Pressure history', 'hPa');
        drawChart('altitudeChart', data.altitude_history, 'Altitude history', 'm');
      } catch (e) {
        console.log('Update error:', e);
      }
    }

    updateData();
    setInterval(updateData, 2000);
  </script>
</body>
</html>
)rawliteral";

  return html;
}

static void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildHTML());
}

static void handleDownload() {
  String fileName = getCurrentLogFileName();

  if (fileName == "") {
    server.send(500, "text/plain", "No log file available");
    return;
  }

  File file = SD.open(fileName, FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  String downloadName = fileName;
  if (downloadName.startsWith("/")) {
    downloadName.remove(0, 1);
  }

  server.sendHeader("Content-Type", "text/csv");
  server.sendHeader("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");
  server.streamFile(file, "text/csv");
  file.close();
}

static void handleData() {
  if (gData == nullptr) {
    server.send(500, "application/json; charset=utf-8", "{\"error\":\"No data\"}");
    return;
  }

  String json = "{";
  json += "\"temperature\":" + String(gData->temperature, 2) + ",";
  json += "\"humidity\":" + String(gData->humidity, 2) + ",";
  json += "\"pressure\":" + String(gData->pressure, 2) + ",";

  if (gData->fixStr == "YES") {
    json += "\"latitude\":" + String(gData->latitude, 6) + ",";
    json += "\"longitude\":" + String(gData->longitude, 6) + ",";
  } else {
    json += "\"latitude\":null,";
    json += "\"longitude\":null,";
  }

  if (gData->gpsAltitudeValid) {
    json += "\"altitude\":" + String(gData->altitude, 2) + ",";
  } else {
    json += "\"altitude\":null,";
  }

  json += "\"date\":\"" + gData->dateStr + "\",";
  json += "\"time_utc\":\"" + gData->timeStr + "\",";
  json += "\"satellites\":" + String(gData->satellites) + ",";
  json += "\"fix\":\"" + gData->fixStr + "\",";
  json += "\"log_file\":\"" + getCurrentLogFileName() + "\",";

  json += "\"pressure_history\":[";
  for (int i = 0; i < historyCount; i++) {
    json += String(pressureHistory[i], 2);
    if (i < historyCount - 1) json += ",";
  }
  json += "],";

  json += "\"altitude_history\":[";
  for (int i = 0; i < historyCount; i++) {
    if (isnan(altitudeHistory[i])) json += "null";
    else json += String(altitudeHistory[i], 2);

    if (i < historyCount - 1) json += ",";
  }
  json += "]";

  json += "}";

  server.send(200, "application/json; charset=utf-8", json);
}

bool initWebUI(MeteoData* dataPtr) {
  gData = dataPtr;

  WiFi.mode(WIFI_AP);
  bool apOk = WiFi.softAP(AP_SSID, AP_PASSWORD);

  if (!apOk) {
    Serial.println("ERROR: softAP start failed");
    return false;
  }

  Serial.println("Wi-Fi AP started");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/download", HTTP_GET, handleDownload);
  server.begin();

  Serial.println("Web server started");
  return true;
}

void handleWebClient() {
  server.handleClient();
}
