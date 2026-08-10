#include "websocket.h"

#include "leds.h"
#include "animations.h"
#include "config.h"
#include <ArduinoJson.h>

static AsyncWebSocket* ws = nullptr;

void broadcastAnimationState();
void broadcastManualState();
static void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len);



void broadcastAnimationState()
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

void broadcastManualState()
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

// Handle socket data traffic
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error) return;

        String type = doc["type"];
        
        if (type == "pattern") {
            setManualMode(false);
            clearManualLeds();
            broadcastManualState();
            String val = doc["value"];

            if (val == "off") {
                stopAnimation();
                clearLeds();
                showLeds();
            } else {
                startAnimation(val);
            }

            broadcastAnimationState();
        } 

        else if (type == "brightness") {
            setAnimationBrightness(doc["value"].as<uint8_t>());
            broadcastAnimationState();
        } 

        else if (type == "speed")
        {
            setAnimationDuration(doc["value"].as<uint16_t>());
            broadcastAnimationState();
        }

        else if (type == "color") {
            String hex = doc["value"].as<String>();
            if (hex.charAt(0) == '#') {
                hex = hex.substring(1);
            }
            long number = strtol(hex.c_str(), NULL, 16);
            setAnimationColor(
                RgbColor(
                    (number >> 16) & 0xFF,
                    (number >> 8) & 0xFF,
                    number & 0xFF));

            // Notify every connected client
            broadcastAnimationState();
        }

        else if(type == "led")
        {
            uint16_t index = doc["index"];
            if(index >= PixelCount)
                return;

            bool state = doc["state"];

            if(state)
            {
                // Manual control takes ownership of the LEDs.
                if(!isManualMode())
                {
                    stopAnimation();
                    clearLeds();
                    showLeds();
                    broadcastAnimationState();
                    setManualMode(true);
                }

                String hex = doc["color"].as<String>();
                if(hex.startsWith("#"))
                    hex.remove(0,1);

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
            broadcastManualState();
        }
        else if(type == "alloff")
        {
            stopAnimation();
            setManualMode(true);
            clearManualLeds();
            showManualLeds();
            broadcastAnimationState();
            broadcastManualState();
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
        {
            if(!isAnimationRunning())
            {
                broadcastAnimationState();
            }
            else
            {
                broadcastAnimationState();
            }
            if(isManualMode())
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
