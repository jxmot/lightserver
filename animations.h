#pragma once

#include <Arduino.h>
#include "pixelstrip.h"

void initAnimations(PixelStrip& strip);

void startAnimation(const String& name);
void stopAnimation();
bool isAnimationRunning();
void updateAnimations();

void setAnimationColor(const RgbColor& color);

void setAnimationBrightness(uint8_t brightness);

void setAnimationDuration(uint16_t duration);

String getAnimationName();

// Current animation state read API.
struct AnimationStateSnapshot
{
    String name;
    RgbColor color;
    uint8_t brightness;
    uint16_t duration;
};

void getAnimationState(AnimationStateSnapshot& snapshot);
