#pragma once

#include <Arduino.h>
#include "pixelstrip.h"

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


struct AnimationStateSnapshot
{
    String name;
    RgbColor color;
    uint8_t brightness;
    uint16_t duration;
};

void getAnimationState(AnimationStateSnapshot& snapshot);
