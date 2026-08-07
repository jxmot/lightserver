#pragma once

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

constexpr uint16_t PixelCount = 4;

using PixelStrip = NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>;

enum class AnimationType : uint8_t
{
    Off,
    TheaterChase,
    Scan,
    ColorFade,
    RainbowCycle,
    FireEffect,
    StarryTwinkle,
    Heartbeat
};

void initAnimations(PixelStrip& strip);

void startAnimation(const String& name);
void stopAnimation();
bool isAnimationRunning();
void updateAnimations();

void setAnimationColor(const RgbColor& color);
RgbColor getAnimationColor();

void setAnimationBrightness(uint8_t brightness);
uint8_t getAnimationBrightness();

void setAnimationDuration(uint16_t duration);
uint16_t getAnimationDuration();

String getAnimationName();
