#pragma once

#include <NeoPixelAnimator.h>
#include "../pixelstrip.h"

namespace FlipFlop
{
    struct State
    {
        PixelStrip* strip;
        RgbColor primaryColor;
        RgbColor secondaryColor;
        uint8_t brightness;
        bool secondaryEnabled;
    };

    // Limits the time between odd/even state changes to the range
    // specified by the animation: 1000 ms down to 100 ms.
    uint16_t normalizeStateDuration(uint16_t stateDuration);

    // Returns the total duration of one complete odd/even cycle.
    uint16_t getAnimationDuration(uint16_t stateDuration);

    // Renders one frame of the FlipFlop animation.
    void render(const State& state, const AnimationParam& param);
}
