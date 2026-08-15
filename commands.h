#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

enum class CommandBroadcastOrder : uint8_t
{
    None,
    AnimationOnly,
    ManualOnly,
    ManualThenAnimation,
    AnimationThenManual
};

struct CommandResult
{
    CommandBroadcastOrder broadcastOrder = CommandBroadcastOrder::None;
};

CommandResult processCommand(JsonDocument& doc);
