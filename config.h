#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// LED hardware configuration
constexpr uint16_t PixelCount = 4;
constexpr uint8_t PixelPin = 16;

extern const char *ssid;
extern const char *password;

#endif
