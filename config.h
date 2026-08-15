#pragma once

#include <Arduino.h>

#define DEBUG_SERVER

// LED hardware configuration
extern const uint16_t PixelCount;
extern const uint8_t PixelPin;

extern const char *ssid;
extern const char *password;

// Maximum time to wait for a WiFi connection, in milliseconds.
extern const uint32_t WiFiConnectionTimeout;

