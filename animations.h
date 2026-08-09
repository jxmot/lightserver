#pragma once

#include <Arduino.h>
#include <NeoPixelBus.h>
#include "leds.h"
#include <NeoPixelAnimator.h>



enum class AnimationType : uint8_t
{
    Ready,
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
