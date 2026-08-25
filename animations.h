#pragma once

#include <Arduino.h>
#include "pixelstrip.h"

void initAnimations(PixelStrip& strip);

void startAnimation(const String& name);
void stopAnimation();
bool isAnimationRunning();
void updateAnimations();

void setAnimationColor(const RgbColor& color);
void setAnimationSecondaryColor(const RgbColor& color);
void setAnimationSecondaryEnabled(bool enabled);
void setAnimationBrightness(uint8_t brightness);
void setAnimationDuration(uint16_t duration);
String getAnimationName();

// Current animation state read API.
struct AnimationStateSnapshot
{
    String name;
    RgbColor color;
    RgbColor secondaryColor;
    bool secondaryEnabled;
    uint8_t brightness;
    uint16_t duration;
    bool secondarySupported;
};

void getAnimationState(AnimationStateSnapshot& snapshot);

struct AnimationInfo
{
    String name;
    String label;
    bool colorSupported;
    bool secondarySupported;
};

size_t getAnimationInfo(AnimationInfo* info, size_t maxCount);
