#include "leds.h"

namespace
{
    PixelStrip strip(PixelCount, PixelPin);

    bool manualMode = false;
    bool manualLedState[PixelCount] = { false };
    RgbColor manualColor[PixelCount] =
    {
        RgbColor(255, 0, 0)
    };
    uint8_t manualBrightness[PixelCount] = { 128 };
}

void initLeds()
{
    strip.Begin();
    strip.ClearTo(RgbColor(0, 0, 0));
    strip.Show();
}

PixelStrip& getLeds()
{
    return strip;
}

void showLeds()
{
    strip.Show();
}

void clearLeds()
{
    strip.ClearTo(RgbColor(0, 0, 0));
}

void setManualMode(bool enabled)
{
    manualMode = enabled;
}

bool isManualMode()
{
    return manualMode;
}

void setManualLed(
    uint16_t index,
    bool state,
    const RgbColor* color,
    const uint8_t* brightness)
{
    if (index >= PixelCount)
        return;

    manualLedState[index] = state;

    if (color)
        manualColor[index] = *color;

    if (brightness)
        manualBrightness[index] = *brightness;
}

bool getManualLedState(uint16_t index)
{
    return index < PixelCount ? manualLedState[index] : false;
}

RgbColor getManualLedColor(uint16_t index)
{
    if (index >= PixelCount)
        return RgbColor(0, 0, 0);

    return manualColor[index];
}

uint8_t getManualLedBrightness(uint16_t index)
{
    return index < PixelCount ? manualBrightness[index] : 0;
}

void clearManualLeds()
{
    for (uint16_t i = 0; i < PixelCount; ++i)
        manualLedState[i] = false;
}
