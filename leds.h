#pragma once

#include <Arduino.h>
#include <NeoPixelBus.h>

constexpr uint16_t PixelCount = 4;
constexpr uint8_t PixelPin = 16;

using PixelStrip = NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>;

void initLeds();
PixelStrip& getLeds();
void showLeds();
void clearLeds();

void setManualMode(bool enabled);
bool isManualMode();

void setManualLed(
    uint16_t index,
    bool state,
    const RgbColor* color = nullptr,
    const uint8_t* brightness = nullptr);

bool getManualLedState(uint16_t index);
RgbColor getManualLedColor(uint16_t index);
uint8_t getManualLedBrightness(uint16_t index);

void clearManualLeds();
