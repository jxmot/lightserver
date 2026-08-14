#pragma once

#include <Arduino.h>
#include "pixelstrip.h"

// LED hardware lifecycle and rendering.
void initLeds();
PixelStrip& getLeds();
void showLeds();
void clearLeds();

// Manual LED state.
void setManualMode(bool enabled);
bool isManualMode();

void setManualLed(
    uint16_t index,
    bool state,
    const RgbColor* color = nullptr,
    const uint8_t* brightness = nullptr);

struct ManualLedState
{
    bool on;
    RgbColor color;
    uint8_t brightness;
};

bool getManualLedState(uint16_t index, ManualLedState& state);
void clearManualLeds();
void showManualLeds();
