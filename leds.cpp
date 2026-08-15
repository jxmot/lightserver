#include "leds.h"

namespace
{
    PixelStrip strip(PixelCount, PixelPin);

    bool manualMode = false;
    bool* manualLedState = nullptr;
    RgbColor* manualColor = nullptr;
    uint8_t* manualBrightness = nullptr;
}

void initLeds()
{
    manualLedState = new bool[PixelCount]();
    manualColor = new RgbColor[PixelCount];
    manualBrightness = new uint8_t[PixelCount];

    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        manualColor[i] = RgbColor(255, 0, 0);
        manualBrightness[i] = 128;
    }

    strip.Begin();
    strip.ClearTo(RgbColor(0, 0, 0));
    strip.Show();
}

PixelStrip& getLeds()
{
    return strip;
}

void showManualLeds()
{
    clearLeds();

    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        if (manualLedState[i])
        {
            RgbColor color = manualColor[i];
            uint8_t brightness = manualBrightness[i];

            strip.SetPixelColor(
                i,
                RgbColor(
                    color.R * brightness / 255,
                    color.G * brightness / 255,
                    color.B * brightness / 255));
        }
    }

    showLeds();
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

bool getManualLedState(uint16_t index, ManualLedState& state)
{
    if (index >= PixelCount)
        return false;

    state.on = manualLedState[index];
    state.color = manualColor[index];
    state.brightness = manualBrightness[index];

    return true;
}

void clearManualLeds()
{
    for (uint16_t i = 0; i < PixelCount; ++i)
        manualLedState[i] = false;
}
