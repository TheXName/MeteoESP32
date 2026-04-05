#ifndef CONFIG_H
#define CONFIG_H

constexpr int SD_CS = 5;

constexpr int GPS_RX_PIN = 16;
constexpr int GPS_TX_PIN = 17;
constexpr int GPS_BAUD = 9600;

constexpr const char* AP_SSID = "MeteoESP32";
constexpr const char* AP_PASSWORD = "12345678";

constexpr unsigned long SENSOR_READ_INTERVAL_MS = 2000;
constexpr unsigned long SD_SAVE_INTERVAL_MS = 5000;
constexpr unsigned long SERIAL_PRINT_INTERVAL_MS = 2000;

#endif
