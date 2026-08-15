#include "animations.h"
#include <NeoPixelAnimator.h>

namespace
{
enum class AnimationType : uint8_t
{
    WifiError,
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

    PixelStrip* strip = nullptr;
    NeoPixelAnimator animationEngine(1);

    AnimationType currentAnimation = AnimationType::Off;

    struct AnimationState
    {
        uint16_t duration = 2000;
        uint8_t brightness = 128;
        RgbColor color = RgbColor(255, 0, 0);
    };

    AnimationState state;

    constexpr uint16_t WifiErrorOnTime = 250;
    constexpr uint16_t WifiErrorOffTime = 500;
    constexpr uint16_t WifiErrorCycleTime = WifiErrorOnTime + WifiErrorOffTime;
    constexpr uint8_t WifiErrorBrightness = 192;

    uint8_t* heat = nullptr;

    RgbColor applyBrightness(const RgbColor& baseColor)
    {
        return RgbColor(
            (baseColor.R * state.brightness) / 255,
            (baseColor.G * state.brightness) / 255,
            (baseColor.B * state.brightness) / 255);
    }

    struct AnimationName
    {
        AnimationType type;
        const char* name;
    };

    constexpr AnimationName animationNames[] =
    {
        { AnimationType::WifiError,      "wifierror" },
        { AnimationType::Ready,          "ready" },
        { AnimationType::TheaterChase,  "chase" },
        { AnimationType::Scan,          "scan" },
        { AnimationType::ColorFade,     "fade" },
        { AnimationType::RainbowCycle,  "rainbow" },
        { AnimationType::FireEffect,    "fire" },
        { AnimationType::StarryTwinkle, "twinkle" },
        { AnimationType::Heartbeat,     "heart" }
    };

    AnimationType animationFromName(const String& name)
    {
        // Temporary test: use the existing "chase" command for WifiError.
        if (name == "chase")
            return AnimationType::WifiError;

        for (const AnimationName& animation : animationNames)
        {
            if (name == animation.name)
                return animation.type;
        }

        return AnimationType::Off;
    }

    const char* animationNameFromType(AnimationType type)
    {
        for (const AnimationName& animation : animationNames)
        {
            if (type == animation.type)
                return animation.name;
        }

        return "off";
    }

    void animationCallback(const AnimationParam& param)
    {
        if (!strip || currentAnimation == AnimationType::Off)
            return;

        if (currentAnimation == AnimationType::WifiError)
        {
            // One cycle is 750 ms: 250 ms red at 3/4 brightness, then
            // 500 ms off. The animation restarts indefinitely below.
            if (param.progress < (static_cast<float>(WifiErrorOnTime) / WifiErrorCycleTime))
                strip->ClearTo(RgbColor(WifiErrorBrightness, 0, 0));
            else
                strip->ClearTo(RgbColor(0, 0, 0));
        }
        else if (currentAnimation == AnimationType::Ready)
        {
            const uint8_t interval = static_cast<uint8_t>(param.progress * 10.0f);

            if ((interval % 2) == 0)
                strip->ClearTo(RgbColor(0, 255, 0));
            else
                strip->ClearTo(RgbColor(0, 0, 0));
        }
        else if (currentAnimation == AnimationType::ColorFade)
        {
            RgbColor targetColor;
            if (param.progress < 0.5f)
            {
                targetColor = RgbColor::LinearBlend(
                    RgbColor(0,0,0),
                    state.color,
                    param.progress / 0.5f);
            }
            else
            {
                targetColor = RgbColor::LinearBlend(
                    state.color,
                    RgbColor(0,0,0),
                    (param.progress - 0.5f) / 0.5f);
            }
            strip->ClearTo(applyBrightness(targetColor));
        }
        else if (currentAnimation == AnimationType::TheaterChase)
        {
            uint16_t pos = (uint16_t)(param.progress * PixelCount);
            if (pos >= PixelCount) pos = PixelCount - 1;

            strip->ClearTo(RgbColor(0,0,0));
            strip->SetPixelColor(pos, applyBrightness(state.color));
        }
        else if (currentAnimation == AnimationType::Scan)
        {
            float t = param.progress * 2.0f;
            if (t > 1.0f)
                t = 2.0f - t;

            uint16_t pos = round(t * (PixelCount - 1));
            strip->ClearTo(RgbColor(0,0,0));
            strip->SetPixelColor(pos, applyBrightness(state.color));
        }
        else if (currentAnimation == AnimationType::RainbowCycle)
        {
            for (uint16_t i = 0; i < PixelCount; i++)
            {
                float hue = param.progress + ((float)i / PixelCount);
                if (hue > 1.0f) hue -= 1.0f;

                strip->SetPixelColor(
                    i,
                    applyBrightness(HslColor(hue, 1.0f, 0.5f)));
            }
        }
        else if (currentAnimation == AnimationType::FireEffect)
        {
            uint8_t steps = _max(1, (uint8_t)(20.0f * param.progress));
            static uint8_t lastStep = 0;

            if (steps != lastStep)
            {
                lastStep = steps;

                for (uint16_t i = 0; i < PixelCount; i++)
                    heat[i] = (heat[i] * 4) / 5;

                if (random(100) < 20)
                {
                    uint16_t idx = random(PixelCount);
                    heat[idx] = _min(
                        255,
                        heat[idx] + random(160, 255));
                }
            }

            for (uint16_t i = 0; i < PixelCount; i++)
            {
                float ratio = heat[i] / 255.0f;

                RgbColor fireColor =
                    RgbColor::LinearBlend(
                        RgbColor(0,0,0),
                        state.color,
                        ratio);

                if (ratio > 0.5f)
                {
                    fireColor =
                        RgbColor::LinearBlend(
                            fireColor,
                            RgbColor(255,255,100),
                            (ratio - 0.5f) * 2.0f);
                }

                strip->SetPixelColor(i, applyBrightness(fireColor));
            }
        }
        else if (currentAnimation == AnimationType::StarryTwinkle)
        {
            uint8_t steps = _max(1, (uint8_t)(20.0f * param.progress));
            static uint8_t lastTwinkleStep = 0;

            if (steps != lastTwinkleStep)
            {
                lastTwinkleStep = steps;

                for (uint16_t i = 0; i < PixelCount; i++)
                {
                    RgbColor c = strip->GetPixelColor(i);
                    strip->SetPixelColor(
                        i,
                        RgbColor(
                            c.R * 0.85f,
                            c.G * 0.85f,
                            c.B * 0.85f));
                }

                if (random(100) < 30)
                {
                    uint16_t star = random(PixelCount);
                    strip->SetPixelColor(
                        star,
                        applyBrightness(
                            RgbColor(
                                random(200,255),
                                random(200,255),
                                255)));
                }
            }
        }
        else if (currentAnimation == AnimationType::Heartbeat)
        {
            float intensity = 0.0f;
            float progress = param.progress;

            if (progress < 0.15f)
                intensity = sin((progress / 0.15f) * PI);
            else if (progress >= 0.25f && progress < 0.40f)
                intensity =
                    sin(((progress - 0.25f) / 0.15f) * PI) * 0.7f;

            RgbColor redBeat(intensity * 255, 0, 0);
            strip->ClearTo(applyBrightness(redBeat));
        }

        if (param.state == AnimationState_Completed)
        {
            if (currentAnimation == AnimationType::Ready)
            {
                currentAnimation = AnimationType::Off;
                strip->ClearTo(RgbColor(0, 0, 0));
                strip->Show();
            }
            else if (currentAnimation != AnimationType::Off)
            {
                animationEngine.RestartAnimation(param.index);
            }
        }
    }
}

void initAnimations(PixelStrip& ledStrip)
{
    randomSeed(esp_random());

    strip = &ledStrip;

    heat = new uint8_t[PixelCount]();
}

void startAnimation(const String& name)
{
    if (!strip)
        return;

    currentAnimation = animationFromName(name);

    if (currentAnimation == AnimationType::Off)
    {
        stopAnimation();
        return;
    }

    uint16_t duration = state.duration;

    if (currentAnimation == AnimationType::WifiError)
        duration = WifiErrorCycleTime;
    else if (currentAnimation == AnimationType::Ready)
        duration = 5000;

    animationEngine.StartAnimation(
        0,
        duration,
        animationCallback);
}

void stopAnimation()
{
    animationEngine.StopAnimation(0);
    currentAnimation = AnimationType::Off;
}

bool isAnimationRunning()
{
    return currentAnimation != AnimationType::Off &&
           animationEngine.IsAnimating();
}

void updateAnimations()
{
    animationEngine.UpdateAnimations();
}

void setAnimationColor(const RgbColor& color)
{
    state.color = color;
}

void setAnimationBrightness(uint8_t brightness)
{
    state.brightness = brightness;
}

void setAnimationDuration(uint16_t duration)
{
    state.duration = duration;

    if (isAnimationRunning())
    {
        animationEngine.StartAnimation(
            0,
            state.duration,
            animationCallback);
    }
}

String getAnimationName()
{
    return animationNameFromType(currentAnimation);
}

void getAnimationState(AnimationStateSnapshot& snapshot)
{
    snapshot.name = getAnimationName();
    snapshot.color = state.color;
    snapshot.brightness = state.brightness;
    snapshot.duration = state.duration;
}
