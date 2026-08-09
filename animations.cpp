#include "animations.h"

namespace
{
    PixelStrip* strip = nullptr;
    NeoPixelAnimator animationEngine(1);

    AnimationType currentAnimation = AnimationType::Off;
    uint16_t animationDuration = 2000;
    uint8_t animationBrightness = 128;
    RgbColor animationColor(255, 0, 0);

    uint8_t* heat = nullptr;

    RgbColor applyBrightness(const RgbColor& baseColor)
    {
        return RgbColor(
            (baseColor.R * animationBrightness) / 255,
            (baseColor.G * animationBrightness) / 255,
            (baseColor.B * animationBrightness) / 255);
    }

    AnimationType animationFromName(const String& name)
    {
        if (name == "ready")   return AnimationType::Ready;
        if (name == "chase")   return AnimationType::TheaterChase;
        if (name == "scan")    return AnimationType::Scan;
        if (name == "fade")    return AnimationType::ColorFade;
        if (name == "rainbow") return AnimationType::RainbowCycle;
        if (name == "fire")    return AnimationType::FireEffect;
        if (name == "twinkle") return AnimationType::StarryTwinkle;
        if (name == "heart")   return AnimationType::Heartbeat;
        return AnimationType::Off;
    }

    void animationCallback(const AnimationParam& param)
    {
        if (!strip || currentAnimation == AnimationType::Off)
            return;

        if (currentAnimation == AnimationType::Ready)
        {
            // Ready lasts five seconds. Divide the normalized animation
            // progress into ten 500 ms intervals and alternate green/off.
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
                    animationColor,
                    param.progress / 0.5f);
            }
            else
            {
                targetColor = RgbColor::LinearBlend(
                    animationColor,
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
            strip->SetPixelColor(pos, applyBrightness(animationColor));
        }
        else if (currentAnimation == AnimationType::Scan)
        {
            float t = param.progress * 2.0f;
            if (t > 1.0f)
                t = 2.0f - t;

            uint16_t pos = round(t * (PixelCount - 1));
            strip->ClearTo(RgbColor(0,0,0));
            strip->SetPixelColor(pos, applyBrightness(animationColor));
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
                        animationColor,
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

    uint16_t duration = animationDuration;

    if (currentAnimation == AnimationType::Ready)
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
    animationColor = color;
}

RgbColor getAnimationColor()
{
    return animationColor;
}

void setAnimationBrightness(uint8_t brightness)
{
    animationBrightness = brightness;
}

uint8_t getAnimationBrightness()
{
    return animationBrightness;
}

void setAnimationDuration(uint16_t duration)
{
    animationDuration = duration;

    if (isAnimationRunning())
    {
        animationEngine.StartAnimation(
            0,
            animationDuration,
            animationCallback);
    }
}

uint16_t getAnimationDuration()
{
    return animationDuration;
}

String getAnimationName()
{
    switch (currentAnimation)
    {
        case AnimationType::Ready:          return "ready";
        case AnimationType::TheaterChase:  return "chase";
        case AnimationType::Scan:          return "scan";
        case AnimationType::ColorFade:     return "fade";
        case AnimationType::RainbowCycle:  return "rainbow";
        case AnimationType::FireEffect:    return "fire";
        case AnimationType::StarryTwinkle: return "twinkle";
        case AnimationType::Heartbeat:     return "heart";
        default:                           return "off";
    }
}
