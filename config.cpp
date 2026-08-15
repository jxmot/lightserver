#include <Arduino.h>

const char *ssid = "YOUR_SSID";
const char *password = "YOUR_SSID_PASSWORD";

// LED hardware configuration
extern const uint16_t PixelCount = 4;
extern const uint8_t PixelPin = 16;

// Maximum time to wait for a WiFi connection, in milliseconds.
extern const uint32_t WiFiConnectionTimeout = 90000;
