#include "websocket.h"

#include "leds.h"
#include "animations.h"
#include "commands.h"
#include "config.h"
#include "protocol.h"
#include <ArduinoJson.h>

static AsyncWebSocket* ws = nullptr;

static void broadcastJson(JsonDocument& doc)
{
    if (ws == nullptr)
        return;

    String output;
    serializeJson(doc, output);
    ws->textAll(output);
}

static void broadcastAnimationState()
{
    StaticJsonDocument<256> doc;

    AnimationStateSnapshot state;
    getAnimationState(state);

    doc["type"] = "animation";
    doc["pattern"] = state.name;

    if (state.name != "rainbow" && state.name != "twinkle")
    {
        char colorString[8];
        sprintf(colorString,
                "#%02X%02X%02X",
                state.color.R,
                state.color.G,
                state.color.B);

        doc["color"] = colorString;
    }

    doc["secondarySupported"] = state.secondarySupported;

    if (state.secondarySupported)
    {
        char secondaryColorString[8];
        sprintf(secondaryColorString,
                "#%02X%02X%02X",
                state.secondaryColor.R,
                state.secondaryColor.G,
                state.secondaryColor.B);

        doc["secondaryColor"] = secondaryColorString;
        doc["secondaryEnabled"] = state.secondaryEnabled;
    }

    doc["brightness"] = state.brightness;
    doc["speed"] = state.duration;

    broadcastJson(doc);
}

static void sendShowInfo(AsyncWebSocketClient *client)
{
    StaticJsonDocument<768> doc;
    doc["type"] = "showInfo";
    doc["version"] = ProtocolVersion::ShowInfo;

    JsonArray shows = doc.createNestedArray("shows");
    AnimationInfo info[16];
    size_t count = getAnimationInfo(info, 16);

    for (size_t i = 0; i < count; i++)
    {
        JsonObject show = shows.createNestedObject();
        show["name"] = info[i].name;
        show["label"] = info[i].label;
        show["colorSupported"] = info[i].colorSupported;
        show["secondarySupported"] = info[i].secondarySupported;
    }

    String output;
    serializeJson(doc, output);
    client->text(output);
}

static void sendLedInfo(AsyncWebSocketClient *client)
{
    StaticJsonDocument<128> doc;
    doc["type"] = "ledInfo";
    doc["version"] = ProtocolVersion::LedInfo;
    doc["count"] = PixelCount;

    String output;
    serializeJson(doc, output);
    client->text(output);
}

static void handleClientInfo(AsyncWebSocketClient *client, JsonDocument& doc)
{
    String page = doc["page"].as<String>();
    uint16_t version = doc["version"] | 0;

    if (page == "shows" && version != ProtocolVersion::ShowInfo)
        sendShowInfo(client);
    else if (page == "leds" && version != ProtocolVersion::LedInfo)
        sendLedInfo(client);
}

static void broadcastManualState()
{
    StaticJsonDocument<1024> doc;
    doc["type"] = "manual";
    JsonArray leds = doc.createNestedArray("leds");
    for(uint16_t i = 0; i < PixelCount; i++)
    {
        JsonObject led = leds.createNestedObject();

        ManualLedState state;
        getManualLedState(i, state);

        led["on"] = state.on;

        char color[8];
        sprintf(
            color,
            "#%02X%02X%02X",
            state.color.R,
            state.color.G,
            state.color.B
        );

        led["color"] = color;
        led["brightness"] = state.brightness;
    }
    broadcastJson(doc);
}

// Handle socket data traffic.
static void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
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

    if (doc["type"] == "clientInfo")
    {
        handleClientInfo(client, doc);
        return;
    }

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
            handleWebSocketMessage(client, arg, data, len);
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
