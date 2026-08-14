#include "websocket.h"

#include "leds.h"
#include "animations.h"
#include "commands.h"
#include "config.h"
#include <ArduinoJson.h>

static AsyncWebSocket* ws = nullptr;

static void broadcastAnimationState()
{
    StaticJsonDocument<256> doc;

    doc["type"] = "animation";
    doc["pattern"] = getAnimationName();

    char colorString[8];
    sprintf(colorString,
            "#%02X%02X%02X",
            getAnimationColor().R,
            getAnimationColor().G,
            getAnimationColor().B);

    doc["color"] = colorString;
    doc["brightness"] = getAnimationBrightness();
    doc["speed"] = getAnimationDuration();

    String output;
    serializeJson(doc, output);

    if (ws != nullptr)
        ws->textAll(output);
}

static void broadcastManualState()
{
    StaticJsonDocument<1024> doc;
    doc["type"] = "manual";
    JsonArray leds = doc.createNestedArray("leds");
    for(uint16_t i = 0; i < PixelCount; i++)
    {
        JsonObject led = leds.createNestedObject();
        led["on"] = getManualLedState(i);
        char color[8];
        sprintf(
            color,
            "#%02X%02X%02X",
            getManualLedColor(i).R,
            getManualLedColor(i).G,
            getManualLedColor(i).B
        );
        led["color"] = color;
        led["brightness"] = getManualLedBrightness(i);
    }
    String output;
    serializeJson(doc, output);
    if (ws != nullptr)
        ws->textAll(output);
}

// Handle socket data traffic.
static void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    if (!info->final ||
        info->index != 0 ||
        info->len != len ||
        info->opcode != WS_TEXT)
    {
        return;
    }

    data[len] = 0;

    StaticJsonDocument<200> doc;

    DeserializationError error = deserializeJson(doc, data);

    if (error)
        return;

    CommandResult result = processCommand(doc);

    switch (result.broadcastOrder)
    {
        case CommandBroadcastOrder::AnimationOnly:
            broadcastAnimationState();
            break;

        case CommandBroadcastOrder::ManualOnly:
            broadcastManualState();
            break;

        case CommandBroadcastOrder::ManualThenAnimation:
            broadcastManualState();
            broadcastAnimationState();
            break;

        case CommandBroadcastOrder::AnimationThenManual:
            broadcastAnimationState();
            broadcastManualState();
            break;

        case CommandBroadcastOrder::None:
            break;
    }
}


static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
        {
            broadcastAnimationState();

            if (isManualMode())
            {
                broadcastManualState();
            }

            break;
        }

        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        default:
            break;
    }
}

void initWebSocket(AsyncWebServer& server)
{
    if (ws == nullptr)
        ws = new AsyncWebSocket("/ws");

    ws->onEvent(onEvent);
    server.addHandler(ws);
}

void cleanupWebSocketClients()
{
    if (ws != nullptr)
        ws->cleanupClients();
}
