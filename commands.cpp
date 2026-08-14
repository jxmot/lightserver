#include "commands.h"

#include "leds.h"
#include "animations.h"
#include "config.h"

static CommandResult handlePatternCommand(JsonDocument& doc)
{
    CommandResult result;
    result.broadcastOrder = CommandBroadcastOrder::ManualThenAnimation;

    setManualMode(false);
    clearManualLeds();

    String value = doc["value"];

    if (value == "off")
    {
        stopAnimation();
        clearLeds();
        showLeds();
    }
    else
    {
        startAnimation(value);
    }

    return result;
}

static CommandResult handleBrightnessCommand(JsonDocument& doc)
{
    setAnimationBrightness(doc["value"].as<uint8_t>());

    CommandResult result;
    result.broadcastOrder = CommandBroadcastOrder::AnimationOnly;
    return result;
}

static CommandResult handleSpeedCommand(JsonDocument& doc)
{
    setAnimationDuration(doc["value"].as<uint16_t>());

    CommandResult result;
    result.broadcastOrder = CommandBroadcastOrder::AnimationOnly;
    return result;
}

static CommandResult handleColorCommand(JsonDocument& doc)
{
    String hex = doc["value"].as<String>();

    if (hex.charAt(0) == '#')
        hex = hex.substring(1);

    long number = strtol(hex.c_str(), NULL, 16);

    setAnimationColor(
        RgbColor(
            (number >> 16) & 0xFF,
            (number >> 8) & 0xFF,
            number & 0xFF));

    CommandResult result;
    result.broadcastOrder = CommandBroadcastOrder::AnimationOnly;
    return result;
}

static CommandResult handleLedCommand(JsonDocument& doc)
{
    CommandResult result;

    uint16_t index = doc["index"];

    if (index >= PixelCount)
        return result;

    bool state = doc["state"];

    if (state)
    {
        if (!isManualMode())
        {
            stopAnimation();
            clearLeds();
            showLeds();
            setManualMode(true);

            result.broadcastOrder =
                CommandBroadcastOrder::AnimationThenManual;
        }

        String hex = doc["color"].as<String>();

        if (hex.startsWith("#"))
            hex.remove(0, 1);

        long number = strtol(hex.c_str(), NULL, 16);

        RgbColor color(
            (number >> 16) & 0xFF,
            (number >> 8) & 0xFF,
            number & 0xFF);

        uint8_t brightness =
            doc["brightness"].as<uint8_t>();

        setManualLed(index, true, &color, &brightness);
    }
    else
    {
        setManualLed(index, false);
    }

    showManualLeds();

    if (result.broadcastOrder == CommandBroadcastOrder::None)
        result.broadcastOrder = CommandBroadcastOrder::ManualOnly;

    return result;
}

static CommandResult handleAllOffCommand()
{
    stopAnimation();
    setManualMode(true);
    clearManualLeds();
    showManualLeds();

    CommandResult result;
    result.broadcastOrder = CommandBroadcastOrder::AnimationThenManual;
    return result;
}

CommandResult processCommand(JsonDocument& doc)
{
    String type = doc["type"];

    if (type == "pattern")
        return handlePatternCommand(doc);

    if (type == "brightness")
        return handleBrightnessCommand(doc);

    if (type == "speed")
        return handleSpeedCommand(doc);

    if (type == "color")
        return handleColorCommand(doc);

    if (type == "led")
        return handleLedCommand(doc);

    if (type == "alloff")
        return handleAllOffCommand();

    return CommandResult{};
}
