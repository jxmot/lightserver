#include "flipflop.h"

namespace FlipFlop
{
    namespace
    {
        constexpr uint8_t FadePercent = 20;
        constexpr uint16_t MinimumStateDuration = 100;
        constexpr uint16_t MaximumStateDuration = 1000;

        RgbColor applyBrightness(const RgbColor& color, uint8_t brightness)
        {
            return RgbColor(
                (color.R * brightness) / 255,
                (color.G * brightness) / 255,
                (color.B * brightness) / 255);
        }

        RgbColor getSecondaryColor(const State& state)
        {
            if (state.secondaryEnabled)
                return state.secondaryColor;

            return RgbColor(0, 0, 0);
        }

        RgbColor blendForProgress(
            const RgbColor& from,
            const RgbColor& to,
            float progress)
        {
            if (progress <= 0.0f)
                return from;

            if (progress >= 1.0f)
                return to;

            return RgbColor::LinearBlend(from, to, progress);
        }
    }

    uint16_t normalizeStateDuration(uint16_t stateDuration)
    {
        if (stateDuration < MinimumStateDuration)
            return MinimumStateDuration;

        if (stateDuration > MaximumStateDuration)
            return MaximumStateDuration;

        return stateDuration;
    }

    uint16_t getAnimationDuration(uint16_t stateDuration)
    {
        stateDuration = normalizeStateDuration(stateDuration);
        return stateDuration * 2;
    }

    void render(const State& state, const AnimationParam& param)
    {
        if (!state.strip)
            return;

        // One complete animation cycle contains two equal state periods:
        // odd LEDs on, then even LEDs on.
        float cycleProgress = param.progress;
        if (cycleProgress < 0.0f)
            cycleProgress = 0.0f;
        if (cycleProgress > 1.0f)
            cycleProgress = 1.0f;

        uint8_t currentState = cycleProgress < 0.5f ? 0 : 1;
        float stateProgress = currentState == 0
            ? cycleProgress * 2.0f
            : (cycleProgress - 0.5f) * 2.0f;

        // The last 20% of each state is the fade to the next state.
        constexpr float fadeStart = 1.0f - (FadePercent / 100.0f);

        float fadeProgress = 0.0f;
        if (stateProgress > fadeStart)
            fadeProgress = (stateProgress - fadeStart) / (1.0f - fadeStart);

        RgbColor secondary = getSecondaryColor(state);

        RgbColor oddFrom;
        RgbColor oddTo;
        RgbColor evenFrom;
        RgbColor evenTo;

        if (currentState == 0)
        {
            oddFrom = state.primaryColor;
            oddTo = secondary;
            evenFrom = secondary;
            evenTo = state.primaryColor;
        }
        else
        {
            oddFrom = secondary;
            oddTo = state.primaryColor;
            evenFrom = state.primaryColor;
            evenTo = secondary;
        }

        // Before the fade, hold the current state. During the fade, each
        // LED group transitions directly to its next color/state.
        float blend = fadeProgress;
        if (stateProgress <= fadeStart)
            blend = 0.0f;

        RgbColor oddColor = applyBrightness(
            blendForProgress(oddFrom, oddTo, blend),
            state.brightness);

        RgbColor evenColor = applyBrightness(
            blendForProgress(evenFrom, evenTo, blend),
            state.brightness);

        for (uint16_t i = 0; i < PixelCount; i++)
        {
            // LED numbers are 1-based in the animation specification.
            // Zero-based index 0 is therefore LED 1 (odd).
            if ((i % 2) == 0)
                state.strip->SetPixelColor(i, oddColor);
            else
                state.strip->SetPixelColor(i, evenColor);
        }
    }
}
